$ErrorActionPreference = 'Stop'
$directory = 'build/windows64/Minecraft.Client/Release'
if (-not (Test-Path "$directory/LegacyRelay.dll")) { throw 'Native relay DLL missing from client output' }
$executable = "$directory/NativeRelaySmoke.exe"
cl /nologo /EHsc /std:c++17 tools/join-code/native/ClientAbiSmoke.cpp "/Fe$executable" /Fologs/native-relay-test.obj 2>&1 | Tee-Object logs/native-relay-test-build.log
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
try {
    & $executable 2>&1 | Tee-Object logs/native-relay-test.log
    if ($LASTEXITCODE -ne 0) { throw "Native relay tests failed ($LASTEXITCODE)" }
} finally {
    if (Test-Path "$directory/join-code.log") { Copy-Item "$directory/join-code.log" logs/native-relay-runtime.log }
    # The smoke executable is not part of the player package.
    Remove-Item $executable -ErrorAction SilentlyContinue
}
