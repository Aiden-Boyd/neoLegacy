# Built-in join codes (old Windows / Steam-Proton client)

The relay was deployed from macOS, but runs on Cloudflare. The hosting computer
and players do not run Wrangler, a terminal companion, or an installed .NET runtime.
LegacyRelay.dll is compiled to native code and automatically packaged with the client.
Keep it beside Minecraft.Client.exe. The endpoint is built in:
wss://legacy-join-relay.newlifemedia.workers.dev.

## Host

1. Create or load an **Online Game** using this build.
2. Pause and choose **Share Join Code**.
3. When connected, that button shows a code such as ABCDE-FGHJK.
4. Share the code and resume playing. Sharing continues after closing the menu.
5. Select the code button again to **Stop sharing**. Exiting the world also stops it.

The primary player's pause-menu Leaderboards slot becomes the sharing control.
Offline worlds are not exposed automatically. Sharing requires an online-enabled
world and an explicit press. Stopping sharing disconnects relay players.

## Join

1. Open **Join Game** and choose **Join with Code**.
2. Enter the host's code using the game's controller or keyboard input.
3. Select the **[Join Code]** world that appears below the action rows.

**Add Server** remains available separately. Relay entries live only in memory;
no server-list or world-save files are rewritten by this integration.
Everyone needs a compatible old game build. This does not add Java/Bedrock support.

## Diagnostics and acceptance

The client writes timestamped lifecycle and failure details to join-code.log beside
the executable, retaining one previous log after 4 MiB. Tokens and game traffic are
not logged. Relay errors appear in the Join Game action row or pause sharing button;
select it again to retry. Missing or unsupported native DLLs show an error without
preventing the rest of the game from starting.

CI builds the actual client and DLL, tests its real C++ loader and native entry points, cancellation,
public host registration, and four simultaneous TCP streams carrying 4 MiB through
the deployed Cloudflare relay. It also smoke-tests the matching dedicated server.
A real two-house gameplay, save/rejoin, controller-menu, and Bazzite/Proton run
still needs acceptance testing; transport tests do not establish those outcomes.

Developers: Windows builds require the .NET 10 SDK and MSVC native toolchain for
NativeAOT. No .NET installation is needed on players' computers. Cross-compilation
must supply a separately Windows-built LegacyRelay.dll beside the client executable.
