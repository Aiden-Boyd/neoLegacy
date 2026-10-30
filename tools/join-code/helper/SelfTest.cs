using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;

static class SelfTest
{
    public static async Task<int> Run(Uri relay)
    {
        using var stop = new CancellationTokenSource(TimeSpan.FromSeconds(90));
        var echo = new TcpListener(IPAddress.Loopback, 0);
        echo.Start();
        var echoes = new List<Task>();
        var echoLoop = Task.Run(async () =>
        {
            try
            {
                while (!stop.IsCancellationRequested)
                {
                    var tcp = await echo.AcceptTcpClientAsync(stop.Token);
                    echoes.Add(Task.Run(async () =>
                    {
                        using (tcp)
                        {
                            var stream = tcp.GetStream(); var buffer = new byte[8192];
                            try
                            {
                                int count;
                                while ((count = await stream.ReadAsync(buffer, stop.Token)) != 0)
                                    await stream.WriteAsync(buffer.AsMemory(0, count), stop.Token);
                            }
                            catch (Exception) when (stop.IsCancellationRequested) { }
                        }
                    }));
                }
            }
            catch (OperationCanceledException) { }
        });
        var code = new TaskCompletionSource<string>(TaskCreationOptions.RunContinuationsAsynchronously);
        var port = new TaskCompletionSource<int>(TaskCreationOptions.RunContinuationsAsynchronously);
        var hostTask = Tunnel.Host(relay, ((IPEndPoint)echo.LocalEndpoint).Port, c => code.SetResult(c), Console.WriteLine, stop.Token);
        Task? guestTask = null;
        try
        {
            await Task.WhenAny(code.Task, hostTask).WaitAsync(stop.Token);
            if (hostTask.IsCompleted) await hostTask;
            guestTask = Tunnel.Guest(relay, await code.Task, p => port.SetResult(p), Console.WriteLine, stop.Token);
            var localPort = await port.Task.WaitAsync(stop.Token);
            await Task.WhenAll(Enumerable.Range(0, 4).Select(async _ =>
            {
                using var tcp = new TcpClient();
                await tcp.ConnectAsync(IPAddress.Loopback, localPort, stop.Token);
                var stream = tcp.GetStream();
                byte[] payload = RandomNumberGenerator.GetBytes(1024 * 1024);
                var send = stream.WriteAsync(payload, stop.Token).AsTask();
                byte[] returned = new byte[payload.Length];
                await stream.ReadExactlyAsync(returned, stop.Token);
                await send;
                if (!payload.SequenceEqual(returned)) throw new IOException("Relayed bytes changed.");
            }));
            var folder = Path.Combine(Path.GetTempPath(), "legacy-list-" + Guid.NewGuid());
            Directory.CreateDirectory(folder);
            try
            {
                ServerList.Add(folder, "ABCDE-FGHJK", localPort);
                ServerList.Add(folder, "KLMNP-QRSTU", localPort);
                using (var reader = new BinaryReader(File.OpenRead(Path.Combine(folder, "servers.db"))))
                {
                    if (reader.ReadUInt32() != 0x5653434D || reader.ReadUInt32() != 1 || reader.ReadUInt32() != 1)
                        throw new IOException("Server-list replacement failed.");
                }
                var path = Path.Combine(folder, "servers.db");
                File.WriteAllBytes(path, [1, 2, 3]);
                try { ServerList.Add(folder, "ABCDE-FGHJK", localPort); throw new Exception("Corrupt list accepted."); }
                catch (IOException) { }
                if (!File.ReadAllBytes(path).SequenceEqual(new byte[] {1, 2, 3})) throw new IOException("Corrupt list was modified.");
            }
            finally { Directory.Delete(folder, recursive: true); }
            Console.WriteLine("PASS: four simultaneous TCP streams, 4 MiB exact echo, flow control, server-list replacement and corrupt-list preservation.");
            return 0;
        }
        finally
        {
            stop.Cancel(); echo.Stop();
            foreach (var task in new[] {hostTask, guestTask, echoLoop}.Where(t => t != null))
                try { await task!; } catch (Exception) when (stop.IsCancellationRequested) { }
            foreach (var task in echoes) try { await task; } catch (Exception) when (stop.IsCancellationRequested) { }
        }
    }
}
