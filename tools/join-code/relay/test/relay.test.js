import {test} from 'node:test';
import assert from 'node:assert/strict';
import {randomBytes} from 'node:crypto';
import WebSocket from 'ws';

const base = process.env.RELAY_TEST_URL || 'ws://127.0.0.1:8787';
function connect(path, token) {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(base + path, token ? {headers: {Authorization: 'Bearer ' + token}} : {});
    const messages = [], readers = [];
    ws.on('message', (data, binary) => {
      const value = {data: binary ? data : data.toString(), binary};
      if (readers.length) readers.shift()(value); else messages.push(value);
    });
    ws.on('error', reject);
    ws.once('open', () => resolve({ws, next: () => messages.length ? Promise.resolve(messages.shift()) :
      new Promise((done, fail) => {
        const timer = setTimeout(() => fail(new Error('Relay message timeout')), 5000);
        readers.push(value => {clearTimeout(timer); done(value);});
      })}));
  });
}
const token = () => randomBytes(32).toString('hex');
const json = message => JSON.parse(message.data);
async function room(code) {
  const secret = token(), host = await connect('/room/' + code + '/host', secret);
  assert.equal(json(await host.next()).type, 'hosted');
  return {host, secret};
}
test('unknown codes and malformed/duplicate host registration are rejected', async () => {
  await assert.rejects(connect('/room/ZZZZZ-ZZZZZ/guest'), /404/);
  await assert.rejects(connect('/room/ABCDE-FGHJK/host', 'bad'), /401/);
  await assert.rejects(connect('/room/INVALID/guest'), /400/);
  const {host} = await room('ABCDE-FGHJK');
  try { await assert.rejects(connect('/room/ABCDE-FGHJK/host', token()), /409/); }
  finally { host.ws.close(); }
});
test('opaque binary bytes, ACK flow control, host authentication and host departure', async () => {
  const code = 'KLMNP-QRSTU', {host, secret} = await room(code);
  const guest = await connect('/room/' + code + '/guest');
  let stream;
  try {
    assert.equal(json(await guest.next()).type, 'waiting');
    const incoming = json(await host.next());
    assert.equal(incoming.type, 'incoming');
    await assert.rejects(connect('/room/' + code + '/stream?id=' + incoming.id, token()), /401/);
    stream = await connect('/room/' + code + '/stream?id=' + incoming.id, secret);
    assert.equal(json(await guest.next()).type, 'ready');
    await assert.rejects(connect('/room/' + code + '/stream?id=' + incoming.id, secret), /404/);
    for (let i = 0; i < 32; i++) {
      const payload = randomBytes(16384);
      guest.ws.send(payload);
      const received = await stream.next();
      assert.equal(received.binary, true); assert.deepEqual(received.data, payload);
      stream.ws.send('ack'); assert.equal((await guest.next()).data, 'ack');
      stream.ws.send(payload);
      assert.deepEqual((await guest.next()).data, payload);
      guest.ws.send('ack'); assert.equal((await stream.next()).data, 'ack');
    }
    const departed = new Promise(resolve => guest.ws.once('close', resolve));
    host.ws.close(); await departed;
    assert.notEqual(guest.ws.readyState, WebSocket.OPEN);
  } finally { host.ws.terminate(); guest.ws.terminate(); stream?.ws.terminate(); }
});
test('a sender exceeding the eight-frame window is disconnected', async () => {
  const code = 'VWXYZ-ABCDE', {host, secret} = await room(code);
  const guest = await connect('/room/' + code + '/guest');
  await guest.next();
  const incoming = json(await host.next());
  const stream = await connect('/room/' + code + '/stream?id=' + incoming.id, secret);
  await guest.next();
  try {
    const closed = new Promise(resolve => guest.ws.once('close', resolve));
    for (let i = 0; i < 9; i++) guest.ws.send(Buffer.alloc(64, i));
    assert.equal(await closed, 1009);
  } finally { host.ws.terminate(); guest.ws.terminate(); stream.ws.terminate(); }
});
