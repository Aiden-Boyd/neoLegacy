# Startup and crash diagnostics

## What the test build records

Every client launch creates `logs/client-DATE-TIME-PID.log` beside the game.
The filename separates sessions and simultaneous processes. If that directory
is not writable, diagnostics try `%TEMP%/neoLegacy-logs` instead.

- Timestamp and thread ID for client debug messages, including Windows Release
  and final-build configurations; existing stdout/stderr output is also saved.
- Build identity from `BUILD_INFO.txt`, startup stages, Direct3D driver attempts,
  HRESULTs and fatal initialization errors.
- Required media/resource directory checks before entering the engine.
- An unhandled Windows exception writes a separate `*-crash.txt` with exception
  code, address, process/thread IDs and a best-effort `.dmp` beside it. An empty
  crash record is normal when the process has not crashed.
- A minidump uses the system's optional `dbghelp.dll`; failure to produce a dump
  must not become a new mandatory dependency or prevent startup.

Logs are flushed immediately. Sessions are retained separately; archive or remove
old sessions when no longer needed. Diagnostics do not log command-line arguments.
They capture existing game messages, which may contain player/server/world names.

Unhandled-exception capture is best effort. Fail-fast termination, external kills,
deadlocks, or a missing imported DLL before the entry point may produce no crash
record. Some direct library `OutputDebugString` calls are not routed through the
client logger. Minidump support depends on the Windows/Proton implementation.

For a Proton launch that fails before a runtime log appears, use Steam launch
options `PROTON_LOG=1 %command%` and collect the generated `steam-*.log` from
your home directory. Preserve the exact build's `BUILD_INFO.txt` with every report.

## Build evidence

The Steam Proton Client workflow retains:

- Verbose configure output, full compiler/linker commands and compiler warnings.
- Toolchain versions, commit, host details, disk space and regression test results.
- CMake cache/configure diagnostics, compile commands and Ninja history when present.
- Matching `.pdb` symbols as a separate artifact for analyzing crash addresses.
- A Windows-only integration test of debug/stdout/stderr logging and a controlled
  child-process exception. This test does not launch or crash the actual game.

Artifacts upload even after a failed build when the files exist. Download the
`steam-proton-build-logs` and `steam-proton-debug-symbols` artifacts from the same
Actions run as the playable build. Their retention is 14 days.

## Startup fixes in this pass

- Preserve depth-buffer and depth-view creation failures instead of overwriting
  them with later HRESULTs and continuing with invalid resources.
- Retry a debug D3D device without its optional debug layer when Graphics Tools
  are missing. Every driver attempt and retry is logged.
- Return failure for window/device initialization errors instead of success.
- Use Unicode executable-directory selection, detect truncated/failed paths, and
  resolve user configuration relative to that directory.
- Return an explicit exit status after the normal game loop.
- Match language-buffer `new[]` allocations with `delete[]` in both legacy
  language-loading paths, avoiding undefined behavior from scalar deletion.
- Keep an owned copy of binary language data in `StringTable`: the caller frees
  its input after construction, but language reload reads the stored data later.

Remaining acceptance: successful Windows CI, then launch, create/load/save/exit
and resize/fullscreen checks on the target Bazzite machine. This source audit
does not establish that all crashes have been found or that gameplay is stable.
