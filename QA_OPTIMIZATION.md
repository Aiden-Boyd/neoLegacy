# QA and optimization checkpoint — 2026-10-28

Base: `7489a14977d359f9e54857f5906a54ee9ae2aefe` on `fix/base-stability`.
Target: faithful Xbox One CU59 behavior, Windows client through Steam/Proton
on Bazzite, original visuals/mechanics and controller support.

## Verified in this pass

- Existing production-source sponge and monument reconstruction regressions pass.
- Sponge absorption now uses bounded, invocation-local storage instead of a
  heap-backed queue/vector. The end-of-node `count > 64` check is preserved:
  at most 70 positions can be absorbed and 71 queue entries include the root.
  This intentionally does not reinterpret the existing absorption limit.
- 1,000 deterministic worlds compare final blocks, metadata, removal counts,
  and callback-visible state against the previous traversal. Cases include
  dense water, sparse pockets, negative/far coordinates, rejected metadata
  writes and reentrant callbacks. Observed maximum removal count: 69.
- A straight water line verifies the existing seven-block traversal reach.
- 10,000 warmed empty attempts: baseline 20,000 heap allocations; candidate 0.
  This fixture result is not an FPS benchmark or proof of CU59 parity.
- g++ with `-O2 -fsanitize=address,undefined -fno-omit-frame-pointer` passes.
  LeakSanitizer cannot inspect processes in this environment, so leak detection
  was disabled (`ASAN_OPTIONS=detect_leaks=0`); address/UB checks remained enabled.

Reproduce:

```sh
python tests/base_stability_regression.py
ASAN_OPTIONS=detect_leaks=0 CXXFLAGS='-O2 -fsanitize=address,undefined -fno-omit-frame-pointer' python tests/sponge_workspace_regression.py
```

The dedicated Steam/Proton build workflow runs both test scripts on MSVC,
then builds and packages the Windows Release client. Compilation and runtime
acceptance are separate gates. Windows CI results must be checked for this commit.

## Remaining release gates

No claim of 4J-equivalent QA or completion is made. This host lacks Windows SDK,
MSVC, Wine/Proton, a running client and the target controller/hardware.

1. Verify the candidate Windows Release build and packaged runtime.
2. Complete every gameplay step in `BASE_STABILITY_TESTS.md`, including
   wet/dry sponge rendering, furnace behavior, host/client synchronization,
   monument reload/guardian spawning and unchanged saved terrain.
3. Verify controller-only menus/gameplay, disconnect/reconnect, focus changes,
   Steam launch/exit, save/reload, and Bazzite suspend/resume.
4. Profile the same copied worlds, seeds, routes, settings, resolution, Proton
   version and hardware against the accepted baseline. Capture frame-time
   distribution (median/p95/p99), loading and saving times, process memory,
   entity-heavy scenes, chunk generation, water updates and redstone load.
   Repeat runs after warm-up; retain raw captures. Set performance budgets from
   the target hardware and reference before declaring a pass.
5. Exercise long sessions, repeated world transitions, multiplayer joins/leaves,
   save failures and copied-world recovery. Confirm no crash or save corruption.
6. Complete a CU59 content/mechanics audit against the actual reference release.
   Preserve world generation and random-call ordering during optimizations;
   require behavior regressions and measured benefit for each further change.

Only checked results should be marked complete. Automated fixtures cannot
establish visual fidelity, multiplayer integration or full save compatibility.
