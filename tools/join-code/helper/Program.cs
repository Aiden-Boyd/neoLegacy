using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Text.Json;

class Program
{
    static async Task<int> Main(string[] args)
    {
        using var stop = new CancellationTokenSource();
        Console.CancelKeyPress += (_, e) => { e.Cancel = true; stop.Cancel(); };
        try
        {
            if (args.Length == 2 && args[0] == "self-test") return await SelfTest.Run(new Uri(args[1]));
            Console.WriteLine("Legacy Join Codes — keep this window open while playing.");
            string directory = AppContext.BaseDirectory;
            string configuration = Path.Combine(directory, "relay.json");
            if (!File.Exists(configuration)) throw new IOException("Missing relay.json. Deploy the relay and configure its wss:// address first.");
            using var settings = JsonDocument.Parse(File.ReadAllText(configuration));
            var relay = new Uri(settings.RootElement.GetProperty("relay").GetString()!);
            var mode = args.FirstOrDefault();
            if (mode == null)
            {
                Console.WriteLine("1. Host and get a code\n2. Join with a code\nClose the game before choosing Join.");
                mode = Console.ReadLine() == "1" ? "host" : "join";
            }
            void Log(string message) => Console.WriteLine($"[{DateTimeOffset.Now:HH:mm:ss}] {message}");
            if (mode == "host")
            {
                Launch(directory, Log);
                Log("Create/load an online-enabled world. Waiting for the game on local port 25565...");
                // A TCP probe may briefly occupy a game connection; it is closed immediately.
                while (!stop.IsCancellationRequested)
                {
                    using var probe = new TcpClient();
                    try { await probe.ConnectAsync(IPAddress.Loopback, 25565, stop.Token); break; }
                    catch (SocketException) { await Task.Delay(1000, stop.Token); }
                }
                await Tunnel.Host(relay, 25565, code => Log("SHARE THIS CODE: " + code), Log, stop.Token);
            }
            else if (mode == "join")
            {
                if (OperatingSystem.IsWindows() && Process.GetProcessesByName("Minecraft.Client").Length != 0)
                    throw new InvalidOperationException("Close the game before preparing a join-code entry, then run Join again.");
                Console.Write("Join code: ");
                string code = Tunnel.Normalize(args.Length > 1 ? args[1] : Console.ReadLine() ?? "");
                await Tunnel.Guest(relay, code, port =>
                {
                    ServerList.Add(directory, code, port);
                    Log("In Join Game, select [Join Code] " + code + ". No public IP is required.");
                    Launch(directory, Log);
                }, Log, stop.Token);
            }
            else throw new ArgumentException("Use host or join CODE.");
            return 0;
        }
        catch (OperationCanceledException) { return 0; }
        catch (Exception e)
        {
            Console.Error.WriteLine("Join-code session failed: " + e.Message);
            return 1;
        }
    }
    static void Launch(string directory, Action<string> log)
    {
        if (!OperatingSystem.IsWindows()) { log("Launch the old client through Steam/Proton from this same folder."); return; }
        var executable = Path.Combine(directory, "Minecraft.Client.exe");
        if (!File.Exists(executable)) throw new FileNotFoundException("Put LegacyJoin beside Minecraft.Client.exe.");
        if (Process.GetProcessesByName("Minecraft.Client").Length != 0) { log("Game is already running; refresh the Join Game list if needed."); return; }
        Process.Start(new ProcessStartInfo(executable) {WorkingDirectory = directory, UseShellExecute = true});
    }
}
