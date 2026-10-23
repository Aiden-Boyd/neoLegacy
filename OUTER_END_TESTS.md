# Outer End islands: Steam/Proton candidate

Branch: `feature/outer-end-islands`, based on the chorus/end-rod candidate. Earlier builds remain on their branches.

## Scope

On large-world builds (including Windows64), this candidate expands the End from 18 to 256 chunks across: a finite 4096 x 4096 block area, from -2048 to 2047 on each horizontal axis. The generator, dimension border, client cache and renderer use the same dimensions. The Overworld and Nether sizes are unchanged. Older console build configurations keep their existing End dimensions.

Terrain uses the existing Perlin density generator with an additional seeded simplex island field beyond the central void gap. It adds larger outer islands, occasional small floating End Stone islands, and mature branching chorus plants on suitable large islands. Population uses the existing per-chunk seeded RNG; features stay within the loaded 2 x 2 population area and do not overwrite existing blocks. Outer decoration does not run the central dragon/spike/podium decorator.

This is a CU59-targeted test candidate, not a claim of exact Xbox One seed or world-size parity. End cities, ships, shulkers and End gateways are not included. For this milestone, explore in Creative flight. Try roughly 1100-1900 blocks from the center in several directions; the island locations depend on the seed. There is deliberately a substantial void gap.

## Saves

Existing chunks continue to load from their saved region coordinates. Terrain is only generated for missing chunks; existing chunks are not regenerated or repopulated. The central density/noise sequence is preserved, and chorus is only placed during outer-chunk population. Use a separate test world or a copy of an existing world for acceptance. Keep the earlier build with its own world copy when comparing candidates.

## Automated validation

`python tests/outer_end_regression.py` compiles the actual Random, ImprovedNoise, PerlinNoise and SimplexNoise implementations, plus production terrain and feature methods. It compares generated block buffers for every chunk of the original 18 x 18 End across four seeds, checks outer land and void in every quadrant, shared density boundaries and generation-order independence, validates chorus support and mature flowers, checks obstruction and client guards, and verifies decoration bounds and determinism. A separate fixture compiles the production dimension size and renderer indexing methods, checking every End section index and boundary rejection. These fixtures do not exercise real storage I/O, GPU rendering or network transport. CI also runs the earlier regressions and compiles the full client.

## In-game acceptance (not yet verified)

1. Create a fresh Creative world, enter the End, and check the central island, dragon, spikes, obsidian entry platform and exit podium against the earlier build using the same seed.
2. Fly beyond the central void gap. Visit several outer islands in positive and negative X/Z, looking for large islands, small floating islands and chorus. Check for chunk seams, unexpected walls, missing terrain or repeated strips. Endermen should use the existing End biome behavior.
3. Harvest fruit and flowers from natural chorus, replant flowers on End Stone, wait for growth, and break stems to check support cascades. Recheck eating/teleporting, smelting, purpur crafting and end rods.
4. Place a recognizable marker and a chest on an outer island. Save, quit fully, reopen, and revisit it. Verify the marker, inventory, natural terrain and player position. Explore into additional chunks after reloading.
5. Repeat on a copy of an older world that has an already visited End. Verify old builds and central terrain remain, then explore new outer chunks and repeat the save/reload check. Do not reset the End as part of this test.
6. Approach all four finite borders and corners. Check terrain rendering, movement limits, block interaction and stability near -2048 and 2047. Test a remote second player observing and interacting with outer chunks.
7. Compare loading time, frame time and memory while exploring and returning to the center. Confirm chunks unload/reload correctly. Recheck the earlier controller, portal-rendering, Elytra/head and sponge fixes through Steam/Proton.

Record seed, coordinates, branch/commit and a Proton log when reporting a crash or terrain problem. The build package contains BUILD_INFO.txt identifying the exact commit.
