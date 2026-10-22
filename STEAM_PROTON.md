# Steam / Proton baseline

This branch first establishes a reproducible Windows client build for testing on Bazzite.
It does not claim Xbox One CU59 parity or verified Steam Controller compatibility.

## Build

The `Steam Proton Client` GitHub Actions workflow builds only `Minecraft.Client`
using the existing Windows64 Release preset. It runs on pushes to
`steam-proton-base` and can also be started manually once available on the default branch.
It uploads a temporary Actions artifact, not a GitHub release.

The workflow deliberately disables recursive submodule checkout: this fork already
contains `Minecraft.Client/Windows64/4JLibs` as ordinary files, while its legacy
`.gitmodules` entry points to an unavailable repository. The workflow checks that
the embedded library build file exists before configuring.

.NET 10 is installed because the current top-level CMake configuration checks for
FourKit even when only the game client is requested. The FourKit server is not
the requested build target.

## Install on Bazzite

1. Download the `neoLegacy-steam-proton-windows64` artifact from a successful run.
2. Extract the complete artifact to a permanent, writable folder.
3. In Steam, use **Games > Add a Non-Steam Game** and browse to `Minecraft.Client.exe`.
4. Open the shortcut's properties. Set **Start In** to the extracted folder that
   contains the executable, `Common`, and the other runtime folders.
5. Under **Compatibility**, force a current installed Proton version. Begin with
   Proton Experimental; record the exact version tested.
6. Under **Controller**, enable Steam Input and begin with a standard Gamepad
   layout. Map the right trackpad to right-stick input as a starting point.
7. Launch through Steam. Keep the game folder writable.

Do not run the repository's Wine launcher inside this Steam shortcut: let Steam
manage the Proton prefix and the game process directly.

## First acceptance check

Record the build commit, Proton version, GPU/driver, desktop or Game Mode session,
and controller model with every test.

- Launch and exit normally; Steam should return to its idle Play state.
- Navigate the title screen and options using only the controller.
- Create a world, move, look, jump, mine, place, and use inventory/crafting.
- Save, exit the game completely, relaunch, and reload the same world.
- Check sound effects, music, fullscreen, focus, and Steam overlay.
- Disconnect and reconnect the controller.
- Check split-screen separately if used.
- Back up the entire `Windows64/GameHDD` directory before replacing a build,
  and verify where this particular build actually writes saves.

A successful compilation is not a gameplay or CU59 parity test. Native Linux,
renderer replacement, and gameplay backports are subsequent work after this
baseline is demonstrated.
