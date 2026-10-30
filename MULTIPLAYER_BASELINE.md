# Old baseline with multiplayer

Game source: steam-proton-base, eaf2da084859e4990a3935c76527d9b07ac9a91e.
Only build packaging/documentation changed. Later gameplay/save/startup patches
are not included.

All players use client/Minecraft.Client.exe from this package. The optional
matching vanilla server is server/Minecraft.Server.exe (no FourKit plugin host).
This port does not connect to stock Java, Bedrock, Realms, or Xbox Live.
Compilation is not proof of a working multiplayer session.

## LAN multiplayer

One player creates/loads an online-enabled, joinable world and stays in it.
Other players look for it in the Join Game list.
Default game traffic: TCP 25565. LAN discovery: UDP 25566.
Allow these through the host firewall for your intended network.
If discovery fails, use direct IP.

## Direct IP

Steam shortcut launch options:
-ip HOST_ADDRESS -port 25565 -name YOUR_NAME

Replace HOST_ADDRESS and YOUR_NAME; give players distinct names.
The existing client parser supports these arguments. Its server-list UI is
another option. The host must have an online-enabled world or server running.

## Dedicated server

Windows: run server/Start-Server.cmd. It runs from its own directory:
Minecraft.Server.exe -port 25565 -bind 0.0.0.0 -name DedicatedServer

Keep the server directory between runs; server.properties is generated there.
Shut down normally through the server console.

Bazzite: add server/Minecraft.Server.exe as a separate non-Steam shortcut,
force Proton, set Start In to the server folder, and use the same arguments
above. This server-through-Proton setup still needs target-machine testing.
The .cmd launcher is for Windows, not a native Linux shell.

The host's client can connect to 127.0.0.1. Other machines use the host's
reachable LAN/private/public address.

## Friends outside the house

Direct IP requires network reachability: forward TCP 25565 to the host or use
a shared private network. Router forwarding alone usually cannot bypass CGNAT.
No Steam friends/invite/relay integration is added.
Use a LAN/private network with trusted players first; internet-facing
authentication and transport have not been audited in this pass.

## Saves and acceptance

Extract to a new writable folder and preserve the old Windows64/GameHDD.
Each extracted client folder has separate saves. Use copies of worlds:
newer-feature worlds may not load correctly in older code.
This rollback includes the old save implementation, not the recent save fixes.

First test a disposable world: save/exit/relaunch, connect two clients,
move/place/break blocks, disconnect/rejoin, restart the host, and confirm
world/player state persists. Keep BUILD_INFO.txt with test results.
