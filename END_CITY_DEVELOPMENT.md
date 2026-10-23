# End city implementation checkpoint

This development branch starts at the successful End gateway candidate. It imports template assets and a tested converter only. It does **not** yet place cities or ships or add shulkers to the game. The playable gateway build remains on `feature/end-gateways`.

## Imported data

`assets/endcity` contains 21 gzip NBT structure templates from the `1.12.2` reference in InventivetalentDev/minecraft-assets. `manifest.json` records the source URLs and Git blob hashes. These are Java reference geometry assets; this import does not establish Xbox One CU59 placement or content parity.

`tools/compile_end_city_templates.py` produces `templates.json`, preserving all 23,150 cells, air, palette properties, tile data, and structure markers. Run `python tools/compile_end_city_templates.py --check` and `python tests/end_city_templates_regression.py` from the repository root. The compressed runs use linear index `(y * sizeZ + z) * sizeX + x`; each pair is `[length, paletteIndex]`.

The ship retains its Elytra marker, two treasure-chest markers, three sentries, dragon head type 5 and brewing stand containing two strong-healing potions. Tower banners retain their base colors and patterns. Structure markers must be consumed by the generator, not placed as playable structure blocks.

## Remaining game integration

1. Map every palette entry and rotation to native tile IDs and metadata, including stairs, pillar axes, slabs, rods, ladders, chests and banners. Add missing dragon-head and banner support before enabling affected templates.
2. Plan connected rooms, towers, bridges and occasional ships deterministically from a local world-seeded random stream. Choose sites on sufficiently high outer-island terrain. Do not change the terrain generator's random sequence.
3. Place only the intersection with the active population chunk. Never read or write unloaded neighboring chunks from a structure piece. Preserve loot, frame and sentry completion across save/reload without duplicating them.
4. Implement shulker attachment, shell animation and armor, targeting, teleportation, projectiles and levitation, plus registry, save data, networking, renderer and spawn-egg integration.
5. Resolve loot and Elytra frames, then build a separate playable candidate with generation-order, chunk-boundary, marker-once, save/reload and combat regressions. Acceptance on Steam/Proton remains required.

The original `StructureFeature` framework has persistent structure saving disabled by default. A city implementation must explicitly account for that before relying on piece state to prevent duplicate sentries or refillable loot.
