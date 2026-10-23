# Base stability test build

This build fixes sponge state isolation, keeps absorption on the host, and includes monuments in the existing
structure-reconstruction path. It does not add content or claim CU59 parity.

## Automated checks

Run `python tests/base_stability_regression.py` with g++/clang++ or from an MSVC
developer shell. GitHub Actions also runs it before building the full client.
The checks compile the actual Sponge.cpp/Sponge.h and the actual monument
reconstruction method against deterministic fixtures. They verify independent
wetness, neighbor updates, particle selection, reload-like initialization,
reentrant callbacks, failed state writes, cross-world isolation, client callbacks,
reconstruction
coordinates, empty block data, and the structures-disabled guard.

Both original implementations fail these regressions; the changed versions pass.
These fixtures do not replace actual chunk loading, rendering or guardian AI tests.

## In-game acceptance on Bazzite

Use a separate extracted game folder and a copy of a test world. Keep the accepted
baseline build available. Use the same Steam/Proton/controller setup as before.

1. In Creative, open Decorations. Dry and wet sponge are listed immediately after
   the five skull variants, before the melon and pumpkins. Check both names and
   textures, then place a dry sponge beside water; it should absorb and become wet.
2. Place another dry sponge elsewhere with no adjacent water. Add water beside
   the first, wet sponge: it must not absorb again or change the second's state.
3. Place a wet sponge elsewhere, then add water beside the second, dry sponge.
   The second must still absorb independently.
4. Check wet sponges drip while dry sponges do not, regardless of placement order.
5. Save, fully exit, relaunch, and repeat the water/neighbor checks.
6. At a known naturally generated monument, record coordinates and guardian
   spawning behavior. Save and restart; revisit the same chunks. The building
   and any player edits must remain intact, and structure-dependent spawning
   should remain consistent. Allow comparable difficulty, light, mob-cap and
   player-distance conditions before comparing spawn results.
7. Repeat with another monument away from the original area. A structures-disabled
   world's reload must not acquire new structures.
8. Confirm controller navigation, ordinary play, saving and clean exit still work.
9. Smelt a wet sponge with coal: the result must be dry sponge. Dry sponge must not
   be accepted as a smelting input. The existing recipe uses metadata 1 to 0.
10. If a second client is available, join a hosted test world and repeat absorption
    and save/reload checks. Both players must see the host's final wet block and
    removed water. The automated client fixture is not a network integration test.

## Creative inventory investigation

The inspected branch already registers both sponge variants in
`Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp`, under Decoration.
`Minecraft.World/Tile.cpp` registers block 19 and a MultiTextureTileItem that
preserves dry/wet metadata. Its placement path preserves that metadata too.
`Sponge.cpp` registers the sponge and sponge_wet texture names, and
`FurnaceRecipes.cpp` already registers wet-to-dry smelting.

No missing Creative registration was found, so no duplicate items were added.
Actual packaged texture rendering and menu visibility still need the in-game
checks above. This does not establish why a particular older executable appeared
to lack sponges. Check BUILD_INFO.txt to identify the candidate being tested.

The monument change calls the existing feature's apply method with empty block
data, matching other structure types. It is not a request to postprocess or
rebuild monument blocks in already saved terrain.
