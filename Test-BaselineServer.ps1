$ErrorActionPreference = "Stop"
$folder = (Resolve-Path "build/windows64/Minecraft.Server/Release").Path
$start = [Diagnostics.ProcessStartInfo]::new()
$start.FileName = Join-Path $folder "Minecraft.Server.exe"
$start.WorkingDirectory = $folder
$start.Arguments = "-port 25565 -bind 127.0.0.1 -name BaselineTest"
$start.UseShellExecute = $false
$start.RedirectStandardInput = $true
$start.RedirectStandardOutput = $true
$start.RedirectStandardError = $true
$process = [Diagnostics.Process]::new()
$process.StartInfo = $start
$out = $null
$err = $null
try {
  if (-not $process.Start()) { throw "Server failed to start" }
  $out = $process.StandardOutput.ReadToEndAsync()
  $err = $process.StandardError.ReadToEndAsync()
  $ready = $false
  for ($attempt = 0; $attempt -lt 45; $attempt++) {
    if ($process.HasExited) { throw "Server exited before listening: $($process.ExitCode)" }
    $listeners = Get-NetTCPConnection -LocalPort 25565 -State Listen -ErrorAction SilentlyContinue
    if ($listeners | Where-Object { $_.OwningProcess -eq $process.Id }) { $ready = $true; break }
    Start-Sleep -Seconds 1
  }
  if (-not $ready) { throw "Server did not listen on TCP 25565 within 45 seconds" }
  $connection = [Net.Sockets.TcpClient]::new()
  try { $connection.Connect("127.0.0.1",25565) } finally { $connection.Dispose() }
  "PASS: matching old server started and accepted a local TCP connection; this is not a gameplay handshake test." | Tee-Object logs/server-listener-test.txt
  $process.StandardInput.WriteLine("stop")
  if (-not $process.WaitForExit(30000)) { throw "Server did not shut down after stop" }
  if ($process.ExitCode -ne 0) { throw "Server stopped with code $($process.ExitCode)" }
} finally {
  if ($process.Id -and -not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
  if ($out) { $out.GetAwaiter().GetResult() | Out-File logs/server-stdout.log }
  if ($err) { $err.GetAwaiter().GetResult() | Out-File logs/server-stderr.log }
  $process.Dispose()
}
