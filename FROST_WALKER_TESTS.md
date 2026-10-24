# Frost Walker candidate

This Windows64 candidate adds Frost Walker and frosted ice to the Mending candidate. It includes previous cities, gateways, shulkers, portable boxes, recoloring, dispenser placement and stability fixes. Use the existing Steam/Proton launch on Bazzite. CU59 parity and in-game acceptance remain unverified.

## Implemented

- Frost Walker enchantment ID 9, rare weight 2, levels I and II, with localized names. Uses the existing Creative enchanted-book list, librarian offers, random enchanted-book loot and treasure-enabled fishing/city enchantment paths. Enchanting tables exclude it.
- Boots applicability and mutual exclusion with Depth Strider in both anvil merge directions. Mending and Unbreaking remain compatible.
- Server living-entity movement checks the boots slot once per occupied-cell change. Feet coordinates use the collision box rather than the legacy player's eye/height offset. Grounded wearers freeze source water one block below within the level's radius, provided the block above is air, the chunk is loaded and placement is unobstructed. Flowing water, lava and covered water remain unchanged.
- Frosted ice block ID 212, ordinary ice friction/translucency, four original crack textures and synchronized age metadata. It starts scheduled thaw checks after 60–120 ticks. Subsequent checks run after 20–40 ticks and use light, age, neighboring ice and the 1-in-3 dense-patch roll. Darkness can preserve the patch until light increases.
- Melting advances direct neighboring ice once rather than recursing through the whole patch. Loss of neighboring ice can melt isolated cells. Melting produces source water, or air in ultra-warm dimensions. The block has no resource drop, Silk Touch recovery or pick-block result, and is not added to Creative.

The subsequent cumulative Nether/beetroot candidate adds magma blocks and Frost Walker hot-floor immunity; see CONTENT_UPDATE_TESTS.md. Complete CU59 parity remains unverified.

## Automated checks

`python tests/frost_walker_regression.py` compiles the actual enchantment classes, frosted-ice implementation, movement hook, native zero-drop ice method and available-enchantment pool against a focused fixture. It exercises both levels; treasure exclusion; incompatibility in both directions; boots-only and once-per-cell selection; water/air/collision/height/client guards; crack stages; light and neighbor rules; scheduling; direct-neighbor propagation; Nether evaporation; invalid metadata clamping and failed writes.

The same script decodes PNGs using Python's standard library, checks all four full-resolution atlas cells against the original sprites, verifies mip hashes, and verifies that every existing atlas pixel outside the four new cells remains unchanged. This does not simulate rendering, actual chunk scheduling, network transport or a full world save.

The full Windows build and previous regressions run in Actions. In-game testing remains required, especially natural movement, thaw timing, client synchronization and save/reload of scheduled blocks.

## In-game acceptance

Use a copied test world and matching builds for multiplayer.

1. Find Frost Walker I and II books in Creative. Apply each to boots with an anvil in Survival. Confirm costs, names, save/reload persistence and controller navigation.
2. Walk from shore over a lake with each level. Level II should cover a wider area. Check that jumping, swimming, flying, holding boots or carrying them in inventory does not freeze water. Try edges, slabs, negative coordinates and chunk boundaries.
3. Check source versus flowing water, water with solid cover, shallow water, water beside other entities and lava. Existing terrain should remain intact.
4. Watch four visible crack stages and water restoration in daylight. Compare a shaded/night patch; illuminate it and confirm thawing resumes. Break ice with normal and Silk Touch tools; it should yield no ice item. Try pick-block.
5. Leave the area, save/reload and return. Ice must retain its age and eventually thaw under suitable light; check against permanent orphan patches.
6. Merge Frost Walker with Depth Strider on boots and books in both input orders. Neither incompatible enchantment may be added. Confirm Mending and Unbreaking still combine normally.
7. Have a guest walk on water and reconnect. Both players should see the same ice, crack stages and thawed water. Confirm no client-only placement.
8. Run MENDING_TESTS.md and the earlier End/shulker-box checks in the same cumulative build. Record BUILD_INFO.txt, seed and coordinates for failures.

## Reference assets and behavior

Four `frosted_ice_0` through `frosted_ice_3` sprites are original 1.12.2 pixels from `InventivetalentDev/minecraft-assets`, ref `1.12.2`, under `assets/minecraft/textures/blocks`. Integrated at terrain row 25, columns 9–12, with matching mip cells. No existing texture cells were replaced. Java 1.12.2 mechanics were consulted as a reference; exact Xbox CU59 behavior needs in-game comparison.
