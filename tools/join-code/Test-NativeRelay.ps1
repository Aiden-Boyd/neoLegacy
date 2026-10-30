$ErrorActionPreference = 'Stop'
$dll = (Resolve-Path 'build/windows64/Minecraft.Client/Release/LegacyRelay.dll').Path
Add-Type @"
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class NativeRelayTest {
    [DllImport(@"$dll", EntryPoint="legacy_host", CallingConvention=CallingConvention.Cdecl)]
    public static extern int Host(int port);
    [DllImport(@"$dll", EntryPoint="legacy_join", CallingConvention=CallingConvention.Cdecl, CharSet=CharSet.Unicode)]
    public static extern int Join(string code);
    [DllImport(@"$dll", EntryPoint="legacy_status", CallingConvention=CallingConvention.Cdecl, CharSet=CharSet.Unicode)]
    public static extern int Status(StringBuilder text, int capacity, out int port);
    [DllImport(@"$dll", EntryPoint="legacy_stop", CallingConvention=CallingConvention.Cdecl)]
    public static extern void Stop();
    [DllImport(@"$dll", EntryPoint="legacy_self_test", CallingConvention=CallingConvention.Cdecl)]
    public static extern int Test();
}
"@
Start-Transcript -Path logs/native-relay-test.log
try {
    $text = [Text.StringBuilder]::new(256)
    $port = 0
    [NativeRelayTest]::Stop()
    if ([NativeRelayTest]::Status($text, 256, [ref]$port) -ne 0 -or $port -ne 0) { throw 'ABI idle snapshot failed' }
    if ([NativeRelayTest]::Join('INVALID') -ne -1) { throw 'Invalid code accepted' }
    if ([NativeRelayTest]::Status($text, 256, [ref]$port) -ne -1) { throw 'Invalid code not exposed to UI' }
    if ([NativeRelayTest]::Host(25565) -ne 1) { throw 'Host command rejected' }
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        Start-Sleep -Milliseconds 100
        $state = [NativeRelayTest]::Status($text, 256, [ref]$port)
    } while ($state -eq 1 -and [DateTime]::UtcNow -lt $deadline)
    if ($state -ne 2 -or $text.ToString() -notmatch '^[A-HJ-NP-Z2-9]{5}-[A-HJ-NP-Z2-9]{5}$') { throw "Native hosting failed: $text" }
    Write-Host 'PASS: native ABI, invalid code rejection, public host registration, UI status/code snapshot.'
    [NativeRelayTest]::Stop()
    if ([NativeRelayTest]::Status($text, 256, [ref]$port) -ne 0 -or $port -ne 0) { throw 'Stop left stale UI state' }
    # Stress cancellation/replacement; old completions must never revive a session.
    for ($i = 0; $i -lt 10; $i++) {
        if ([NativeRelayTest]::Join('ABCDE-FGHJK') -ne 1) { throw 'Join start failed' }
        [NativeRelayTest]::Stop()
    }
    Start-Sleep -Milliseconds 250
    if ([NativeRelayTest]::Status($text, 256, [ref]$port) -ne 0 -or $port -ne 0) { throw 'Cancelled session revived' }
    if ([NativeRelayTest]::Test() -ne 0) { throw 'Native public TCP transport self-test failed' }
    Write-Host 'PASS: native DLL four-stream 4 MiB exact public TCP echo, cancellation/replacement, and list preservation.'
} finally {
    [NativeRelayTest]::Stop()
    if (Test-Path 'build/windows64/Minecraft.Client/Release/join-code.log') {
        Copy-Item 'build/windows64/Minecraft.Client/Release/join-code.log' logs/native-relay-runtime.log
    }
    Stop-Transcript
}
