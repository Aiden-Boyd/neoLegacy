using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// A native library loaded into Minecraft.Client. No process, installed runtime,
// configuration file, save writes or server-list writes are needed.
static class Exports
{
    static readonly object Gate = new();
    static readonly Uri Relay = new("wss://legacy-join-relay.newlifemedia.workers.dev");
    static CancellationTokenSource? session;
    static long generation;
    static int state, port;
    static string label = "Join Codes";

    static void Log(string message)
    {
        try
        {
            string path = Path.Combine(AppContext.BaseDirectory, "join-code.log");
            // Bound diagnostics; never log bearer tokens or world bytes.
            lock (Gate)
            {
                if (File.Exists(path) && new FileInfo(path).Length > 4 * 1024 * 1024)
                    File.Move(path, path + ".previous", overwrite: true);
                File.AppendAllText(path, $"[{DateTimeOffset.Now:O}] {message}{Environment.NewLine}");
            }
        }
        catch { /* A read-only log directory must not stop the game. */ }
    }
    static void Update(long id, int value, string text, int localPort = 0)
    {
        lock (Gate)
        {
            if (generation != id) return;
            state = value; label = text; port = localPort;
        }
    }
    static void StopCore()
    {
        CancellationTokenSource? previous;
        lock (Gate)
        {
            ++generation; previous = session; session = null;
            state = 0; port = 0; label = "Join Codes";
        }
        // Cancel outside the state lock; cancellation callbacks may finish on
        // another thread. Disposal belongs to the session's finally block.
        try { previous?.Cancel(); } catch (ObjectDisposedException) { }
        Log("Session stopped.");
    }
    static int Start(string? code, int hostPort)
    {
        StopCore();
        if (code != null)
        {
            try { code = Tunnel.Normalize(code); }
            catch (Exception e) { lock (Gate) { state = -1; label = e.Message; } return -1; }
        }
        else if (hostPort < 1 || hostPort > 65535) return -1;
        var lifetime = new CancellationTokenSource();
        long id;
        lock (Gate)
        {
            id = ++generation; session = lifetime; state = 1;
            label = "Connecting to relay...";
        }
        _ = Task.Run(async () =>
        {
            try
            {
                Log(code == null ? "Starting host relay." : "Starting guest relay.");
                void SessionLog(string message)
                {
                    Log(message);
                    if (message.StartsWith("Join failed/ended:"))
                        Update(id, -1, "Join failed. Check code / host.");
                }
                if (code == null)
                    await Tunnel.Host(Relay, hostPort, ready => Update(id, 2, ready), SessionLog, lifetime.Token);
                else
                    await Tunnel.Guest(Relay, code, ready => Update(id, 3, "[Join Code] " + code, ready), SessionLog, lifetime.Token);
                if (!lifetime.IsCancellationRequested) Update(id, -1, "Relay disconnected. Retry.");
            }
            catch (OperationCanceledException) when (lifetime.IsCancellationRequested) { }
            catch (Exception e)
            {
                Log("Relay error: " + e);
                Update(id, -1, "Relay unavailable. Retry.");
            }
            finally
            {
                // Detach before disposal so Stop cannot cancel a disposed CTS.
                lock (Gate) { if (ReferenceEquals(session, lifetime)) session = null; }
                lifetime.Dispose();
                Log("Session cleanup complete.");
            }
        });
        return 1;
    }

    [UnmanagedCallersOnly(EntryPoint = "legacy_host", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int Host(int localPort)
    { try { return Start(null, localPort); } catch (Exception e) { Log(e.ToString()); return -1; } }

    [UnmanagedCallersOnly(EntryPoint = "legacy_join", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static unsafe int Join(char* code)
    {
        try
        {
            if (code == null) return -1;
            int length = 0;
            while (length < 32 && code[length] != 0) ++length;
            return Start(new string(code, 0, length), 0);
        }
        catch (Exception e) { Log(e.ToString()); return -1; }
    }

    // One locked snapshot keeps status, text and port consistent across the ABI.
    [UnmanagedCallersOnly(EntryPoint = "legacy_status", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static unsafe int Status(char* text, int capacity, int* localPort)
    {
        try
        {
            lock (Gate)
            {
                if (localPort != null) *localPort = port;
                if (text != null && capacity > 0)
                {
                    int count = Math.Min(label.Length, capacity - 1);
                    for (int i = 0; i < count; ++i) text[i] = label[i];
                    text[count] = '\0';
                }
                return state;
            }
        }
        catch { return -1; }
    }
    [UnmanagedCallersOnly(EntryPoint = "legacy_stop", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void Stop() { try { StopCore(); } catch (Exception e) { Log(e.ToString()); } }

    [UnmanagedCallersOnly(EntryPoint = "legacy_self_test", CallConvs = new[] { typeof(CallConvCdecl) })]
    public static int Test()
    {
        try { return SelfTest.Run(Relay).GetAwaiter().GetResult(); }
        catch (Exception e) { Log("Native self-test: " + e); return 1; }
    }
}
