# World creation with texture packs

Create and Load used an unchecked cast from TexturePack to DLCTexturePack before
checking its license owner. Local folder/file packs do not have that object
layout, so choosing one could dereference invalid data before loading starts.
The menus now query a virtual base method: default/local packs return no DLC
owner, while DLCTexturePack retains its existing ownership and trial checks.
Missing pack pointers and a previously selected DLC owner are handled safely.

The automated regression exercises both production lookup statements and the
base getter for default/local packs, purchased/trial DLC, missing packs and pack
switching. A full Windows build is also required. This does not prove the reported
crash fixed without reproducing it with the user's chosen pack under Proton.

Manual checks:

1. Start fresh and create a Survival world with the default pack.
2. Repeat with the pack that crashed; test Creative and a flat world too.
3. Save, quit and reload each world with its selected pack.
4. Switch packs before creating/loading and check that missing packs show their
   existing warning; purchased DLC must retain its normal license behavior.
5. If it still crashes, reproduce with Steam launch options
   `PROTON_LOG=1 %command%` and retain the new `steam-*.log` in your home folder.
   Report the pack name and whether a loading screen appeared.
