const OPEN = 1;
const CODE = /^[A-HJ-NP-Z2-9]{10}$/;
const MAX_GUESTS = 8;
const WINDOW = 8;
const FRAME = 16384;

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    if (url.pathname === '/health') return new Response('legacy-code-relay-v1');
    const match = /^\/room\/([A-Z2-9-]+)\/(host|guest|stream)$/.exec(url.pathname);
    if (!match) return new Response('Not found', {status: 404});
    const code = match[1].replaceAll('-', '');
    if (!CODE.test(code)) return new Response('Invalid code', {status: 400});
    if (request.headers.get('Upgrade')?.toLowerCase() !== 'websocket')
      return new Response('WebSocket required', {status: 426});
    return env.ROOMS.get(env.ROOMS.idFromName(code)).fetch(request);
  }
};

export class Room {
  constructor(ctx) { this.ctx = ctx; }
  sockets(role) { return this.ctx.getWebSockets(role).filter(ws => ws.readyState === OPEN); }
  host() { return this.sockets('control')[0]; }
  peer(ws) {
    const a = ws.deserializeAttachment();
    return this.sockets('data').find(other => other !== ws && other.deserializeAttachment().id === a.id);
  }
  async fetch(request) {
    const url = new URL(request.url);
    const role = url.pathname.split('/').at(-1);
    const control = this.host();
    let attachment;
    if (role === 'host') {
      if (control) return new Response('Room occupied', {status: 409});
      const token = request.headers.get('Authorization') || '';
      if (!/^Bearer [a-f0-9]{64}$/.test(token)) return new Response('Host token required', {status: 401});
      // A token is retained only in the host socket's hibernation attachment.
      // Remove orphaned streams before reusing a disconnected room code.
      for (const old of this.sockets('data')) old.close(1012, 'Session replaced');
      attachment = {role: 'control', token, session: crypto.randomUUID()};
    } else if (role === 'guest') {
      if (!control) return new Response('Host offline or unknown code', {status: 404});
      if (this.sockets('guest').length >= MAX_GUESTS) return new Response('Room full', {status: 429});
      attachment = {role: 'guest', id: crypto.randomUUID(), sent: 0, session: control.deserializeAttachment().session};
    } else if (role === 'stream') {
      if (!control || request.headers.get('Authorization') !== control.deserializeAttachment().token)
        return new Response('Unauthorized', {status: 401});
      const id = url.searchParams.get('id');
      const guest = this.sockets('guest').find(ws => ws.deserializeAttachment().id === id);
      if (!guest || guest.deserializeAttachment().session !== control.deserializeAttachment().session || this.peer(guest))
        return new Response('Unknown or already paired stream', {status: 404});
      attachment = {role: 'stream', id, sent: 0, session: guest.deserializeAttachment().session};
    } else return new Response('Not found', {status: 404});

    const pair = new WebSocketPair();
    const client = pair[0], server = pair[1];
    const tags = attachment.role === 'control' ? ['control'] : ['data', attachment.role];
    this.ctx.acceptWebSocket(server, tags);
    server.serializeAttachment(attachment);
    if (role === 'host') server.send(JSON.stringify({type: 'hosted'}));
    if (role === 'guest') {
      server.send(JSON.stringify({type: 'waiting'}));
      control.send(JSON.stringify({type: 'incoming', id: attachment.id}));
    }
    if (role === 'stream') this.peer(server).send(JSON.stringify({type: 'ready'}));
    return new Response(null, {status: 101, webSocket: client});
  }
  webSocketMessage(ws, message) {
    const a = ws.deserializeAttachment();
    if (a.role === 'control') {
      if (message === 'ping') { ws.send('pong'); return; }
      ws.close(1008, 'Unexpected control message'); return;
    }
    const peer = this.peer(ws);
    if (!peer) { ws.close(1008, 'No paired stream'); return; }
    const p = peer.deserializeAttachment();
    if (typeof message === 'string') {
      if (message !== 'ack' || p.sent <= 0) { ws.close(1008, 'Invalid acknowledgement'); return; }
      p.sent--;
      peer.serializeAttachment(p);
    } else {
      if (!(message instanceof ArrayBuffer) || message.byteLength > FRAME || a.sent >= WINDOW) {
        ws.close(1009, 'Flow-control limit'); return;
      }
      a.sent++;
      ws.serializeAttachment(a);
    }
    peer.send(message);
  }
  webSocketClose(ws) { this.disconnect(ws); }
  webSocketError(ws) { this.disconnect(ws); }
  disconnect(ws) {
    const a = ws.deserializeAttachment();
    if (a?.role === 'control') {
      for (const other of this.sockets('data'))
        if (other.deserializeAttachment().session === a.session) other.close(1012, 'Host left');
    } else if (a) {
      this.peer(ws)?.close(1000, 'Peer left');
    }
  }
}
