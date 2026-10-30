using System.Net;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

static class Tunnel
{
    const string Alphabet = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    public static string Normalize(string code)
    {
        code = code.Replace("-", "").Trim().ToUpperInvariant();
        if (!Regex.IsMatch(code, "^[A-HJ-NP-Z2-9]{10}$")) throw new ArgumentException("Enter the 10-character join code.");
        return code;
    }
    static Uri Endpoint(Uri relay, string code, string route, string? id = null)
    {
        if (relay.Scheme != "wss" && !(relay.Scheme == "ws" && relay.IsLoopback))
            throw new ArgumentException("Use wss:// for a public relay; ws:// is allowed only for local tests.");
        return new Uri(relay, $"/room/{code}/{route}" + (id == null ? "" : "?id=" + Uri.EscapeDataString(id)));
    }
    static ClientWebSocket Socket(string? token = null)
    {
        var socket = new ClientWebSocket();
        socket.Options.KeepAliveInterval = TimeSpan.FromSeconds(20);
        socket.Options.KeepAliveTimeout = TimeSpan.FromSeconds(20);
        if (token != null) socket.Options.SetRequestHeader("Authorization", "Bearer " + token);
        return socket;
    }
    static async Task<string> Text(WebSocket socket, CancellationToken ct)
    {
        var buffer = new byte[4096];
        int count = 0;
        WebSocketReceiveResult received;
        do
        {
            received = await socket.ReceiveAsync(new ArraySegment<byte>(buffer, count, buffer.Length - count), ct);
            if (received.MessageType == WebSocketMessageType.Close) throw new IOException("Relay session closed.");
            count += received.Count;
            if (received.MessageType != WebSocketMessageType.Text || (!received.EndOfMessage && count == buffer.Length))
                throw new IOException("Invalid relay control message.");
        } while (!received.EndOfMessage);
        return Encoding.UTF8.GetString(buffer, 0, count);
    }
    public static async Task Host(Uri relay, int port, Action<string> ready, Action<string> log, CancellationToken ct)
    {
        string code = new(Enumerable.Range(0, 10).Select(_ => Alphabet[RandomNumberGenerator.GetInt32(Alphabet.Length)]).ToArray());
        string token = Convert.ToHexString(RandomNumberGenerator.GetBytes(32)).ToLowerInvariant();
        using var session = CancellationTokenSource.CreateLinkedTokenSource(ct);
        using var control = Socket(token);
        await control.ConnectAsync(Endpoint(relay, code, "host"), ct);
        using (var initial = JsonDocument.Parse(await Text(control, ct)))
            if (initial.RootElement.GetProperty("type").GetString() != "hosted") throw new IOException("Host registration failed.");
        ready(code[..5] + "-" + code[5..]);
        var tasks = new List<Task>();
        try
        {
            while (!session.IsCancellationRequested)
            {
                using var message = JsonDocument.Parse(await Text(control, session.Token));
                if (message.RootElement.GetProperty("type").GetString() != "incoming") continue;
                var id = message.RootElement.GetProperty("id").GetString()!;
                tasks.RemoveAll(t => t.IsCompleted);
                tasks.Add(HostStream(relay, code, token, id, port, log, session.Token));
            }
        }
        finally
        {
            session.Cancel();
            control.Abort();
            await Task.WhenAll(tasks);
        }
    }
    static async Task HostStream(Uri relay, string code, string token, string id, int port, Action<string> log, CancellationToken ct)
    {
        try
        {
            using var lifetime = CancellationTokenSource.CreateLinkedTokenSource(ct);
            lifetime.CancelAfter(TimeSpan.FromSeconds(20));
            using var tcp = new TcpClient();
            await tcp.ConnectAsync(IPAddress.Loopback, port, lifetime.Token);
            using var socket = Socket(token);
            await socket.ConnectAsync(Endpoint(relay, code, "stream", id), lifetime.Token);
            lifetime.CancelAfter(Timeout.InfiniteTimeSpan);
            log("Player tunnel connected.");
            await Bridge(tcp, socket, lifetime.Token);
        }
        catch (Exception e) when (e is IOException or WebSocketException or SocketException or OperationCanceledException)
        { if (!ct.IsCancellationRequested) log("Player tunnel ended: " + e.Message); }
    }
    public static async Task Guest(Uri relay, string code, Action<int> ready, Action<string> log, CancellationToken ct)
    {
        code = Normalize(code);
        var listener = new TcpListener(IPAddress.Loopback, 0);
        listener.Start();
        using var session = CancellationTokenSource.CreateLinkedTokenSource(ct);
        var tasks = new List<Task>();
        try
        {
            ready(((IPEndPoint)listener.LocalEndpoint).Port);
            while (!ct.IsCancellationRequested)
            {
                var tcp = await listener.AcceptTcpClientAsync(ct);
                tasks.RemoveAll(t => t.IsCompleted);
                tasks.Add(GuestStream(tcp, relay, code, log, session.Token));
            }
        }
        finally
        {
            listener.Stop();
            session.Cancel();
            await Task.WhenAll(tasks);
        }
    }
    static async Task GuestStream(TcpClient tcp, Uri relay, string code, Action<string> log, CancellationToken ct)
    {
        using (tcp)
        using (var socket = Socket())
        using (var lifetime = CancellationTokenSource.CreateLinkedTokenSource(ct))
        {
            try
            {
                lifetime.CancelAfter(TimeSpan.FromSeconds(20));
                await socket.ConnectAsync(Endpoint(relay, code, "guest"), lifetime.Token);
                while (true)
                {
                    using var message = JsonDocument.Parse(await Text(socket, lifetime.Token));
                    if (message.RootElement.GetProperty("type").GetString() == "ready") break;
                }
                lifetime.CancelAfter(Timeout.InfiniteTimeSpan);
                log("Connected through join-code relay.");
                await Bridge(tcp, socket, lifetime.Token);
            }
            catch (Exception e) when (e is IOException or WebSocketException or SocketException or OperationCanceledException)
            { if (!ct.IsCancellationRequested) log("Join failed/ended: " + e.Message); }
        }
    }
    public static async Task Bridge(TcpClient tcp, WebSocket socket, CancellationToken ct)
    {
        using var stop = CancellationTokenSource.CreateLinkedTokenSource(ct);
        using var window = new SemaphoreSlim(8, 8);
        using var sendLock = new SemaphoreSlim(1, 1);
        var stream = tcp.GetStream();
        async Task Send(ArraySegment<byte> bytes, WebSocketMessageType type)
        {
            await sendLock.WaitAsync(stop.Token);
            try { await socket.SendAsync(bytes, type, true, stop.Token); }
            finally { sendLock.Release(); }
        }
        async Task Up()
        {
            var buffer = new byte[16384];
            while (true)
            {
                await window.WaitAsync(stop.Token);
                int count = await stream.ReadAsync(buffer, stop.Token);
                if (count == 0) return;
                await Send(new ArraySegment<byte>(buffer, 0, count), WebSocketMessageType.Binary);
            }
        }
        async Task Down()
        {
            var buffer = new byte[16384];
            while (true)
            {
                int count = 0;
                WebSocketReceiveResult result;
                do
                {
                    result = await socket.ReceiveAsync(new ArraySegment<byte>(buffer, count, buffer.Length - count), stop.Token);
                    if (result.MessageType == WebSocketMessageType.Close) return;
                    count += result.Count;
                    if (!result.EndOfMessage && count == buffer.Length) throw new IOException("Oversized relay frame.");
                } while (!result.EndOfMessage);
                if (result.MessageType == WebSocketMessageType.Text)
                {
                    if (Encoding.UTF8.GetString(buffer, 0, count) != "ack") throw new IOException("Invalid flow-control acknowledgement.");
                    window.Release();
                }
                else
                {
                    await stream.WriteAsync(buffer.AsMemory(0, count), stop.Token);
                    await Send(new ArraySegment<byte>(Encoding.UTF8.GetBytes("ack")), WebSocketMessageType.Text);
                }
            }
        }
        var up = Up(); var down = Down();
        try { await await Task.WhenAny(up, down); }
        finally
        {
            stop.Cancel(); socket.Abort(); tcp.Close();
            try { await Task.WhenAll(up, down); }
            catch (Exception) when (stop.IsCancellationRequested) { }
        }
    }
}
