# Nether, bone and beetroot candidate

Cumulative Windows64 build, based on the Frost Walker candidate. Includes the earlier Mending, End/city/shulker, portable-box and stability work. Use the existing Steam/Proton entry on Bazzite. Compilation and fixtures do not establish complete Xbox One CU59 parity or in-game acceptance.

## Added behavior

- Magma block (213): 0.5 hardness, pickaxe harvesting, low emitted light and full-bright surface shading. A server contact check applies hot-floor damage while standing still as well as walking. Sneaking, fire immunity and Frost Walker boots prevent it; the fire-type damage source uses the engine's existing Fire Resistance, armor and hurt-cooldown rules. Separate localized death messages support ordinary deaths and kill credit. Non-fire-immune mobs are rejected by the generic spawn check when standing on magma.
- Magma evaporates source or flowing water immediately above it on block ticks, with fizz and smoke. It does not add later-version bubble columns. The original three-frame texture is routed through the native animation loader with eight ticks per frame.
- Four magma ore attempts per new Nether population chunk, size 33, centered at Y 27–36, replace netherrack only. A separate seeded random stream leaves the previous population RNG sequence unchanged. Existing populated chunks are not regenerated. Magma placements request lighting updates; older ore placements keep their existing flags. Exact Java/Xbox vein locations are not promised.
- Nether wart block (214): nine wart in a 3×3 recipe; no reverse unpacking recipe. Axes mine it faster. Red Nether bricks (215): diagonal two wart/two Nether brick items in a 2×2 recipe.
- Bone block (216): nine bone meal, reversible into nine bone meal. Six placement faces select the pillar axis, synchronized using the native metadata mask. Side/end textures differ; normal and Silk Touch drops lose orientation metadata and stack normally. Fossil structures are not part of this batch.
- Beetroot crop (207), root (434), seeds (435), soup (436): planting on farmland, four growth/selection stages, reduced natural-growth opportunity and bone-meal increments, mature roots plus seed rolls including Fortune, immature seeds, native food consumption, and unstackable six-hunger soup returning a bowl. One root makes red dye; six roots plus a bowl make soup. Farmland recognizes the new crop, bone meal stops at maturity, and pick-block returns seeds.
- Pigs accept/follow beetroot and potatoes, and chickens accept/follow beetroot seeds. New End-city treasure includes plantable seeds with the reference weight 5 and count 1–10; old chests are not refilled. All four building blocks and all three inventory items appear in Creative. Farmer-villager beetroot work/trades and village crop generation are separate remaining work.

## Shared fixes required by this content

The shapeless matcher now removes one actual remaining ingredient per input slot. This enables recipes with repeated ingredients, including beetroot soup, and rejects extra copies. The real recipe parsers now read promoted wide-character varargs as `int`, then convert to `wchar_t`, which also allows the parser to run correctly under GCC. Existing box recoloring regressions exercise the corrected matcher.

Seed placement checks empty stacks and only consumes a seed after a successful native block write. Permission, top-face, soil, space and preview behavior are preserved and covered for beetroot, wheat and wart configurations.

The fixed atlases use terrain row 26 columns 0–8 and previously unused item row 12 columns 1–3. All earlier texture pixels are preserved at each mip level. The historical Frost Walker atlas snapshot is updated for these intentional additions; its four original stage hashes remain intact.

## Automated verification

`python tests/content_update_regression.py` compiles the production block classes, crop implementation, generic age/meal hooks, magma contact/spawn checks, seed placement, bowl consumption and animal food predicates against a focused fixture. It additionally runs:

- Actual new recipe registration calls through the native shaped/shapeless vararg parsers, plus the production shapeless matcher: ingredient counts/metadata, repeat ingredients, missing ingredients, excess ingredients and varied grid positions.
- The real ore-placement code plus the appended population block: repeated seeds/coordinates, negative chunks, loaded-chunk guards, netherrack-only replacement, ordinary-ore flags and magma light-update flags. The fixture RNG tests deterministic integration; it does not certify exact native seeded locations.
- PNG pixel/mip hashes, sprite placement, preservation outside new cells, and the original three animation frames/timing definitions.

All previous regression steps and full Windows compilation run in Actions. Fixtures do not simulate real network transport, GPU animation, all AI goals, actual world save files, or full controller UI navigation. Map-color parity, village/fossil generation and other remaining CU59 gaps are not certified by this batch.

## In-game combined acceptance

Back up the save and use a copied test world. Both multiplayer players must use this cumulative build.

1. Find magma, wart blocks, red Nether bricks and bone blocks in Creative. Check names, held/dropped appearance, placed textures and the animated magma surface. Check all beetroot item icons.
2. Craft every new recipe in both crafting interfaces using the controller. Verify exact ingredients consumed. Soup needs six distinct beetroot input slots plus a bowl in classic crafting; missing ingredients and extra items must fail. Recheck filled/named box recolors.
3. Stand still, walk, sneak and jump on magma in Survival. Check ordinary damage/cooldown, armor, Fire Resistance and Frost Walker boots. Holding Frost Walker boots without wearing them must not protect. Check ordinary and kill-credit death messages.
4. Place source and flowing water directly above magma and nearby. Only the directly covered water should evaporate. Check light, smoke, sounds and guest synchronization. No bubble column should appear.
5. Enter newly generated Nether chunks and look near lava level for magma patches. Save/reload, cross population boundaries and check lighting. Existing explored chunks should not be repopulated.
6. Place bone blocks against all six faces. Save/reload and have a guest observe the same orientation. Break with normal and Silk Touch pickaxes; drops should stack together. Unpack into nine bone meal. Check pickaxe/axe speeds and expected drops on all new blocks.
7. Plant beetroot on farmland; try wrong faces, blocked space, wrong soil and controller placement previews. Watch all four stages, apply bone meal, and harvest immature/mature/Fortune crops. Verify dry planted farmland is not prematurely replaced with dirt. Save/reload at each stage and test guest planting/harvesting for duplicates.
8. Eat raw beetroot and soup with hunger missing. Confirm nutrition and bowl return; soup must not stack. Breed/tempt pigs with beetroot or potatoes and chickens with beetroot seeds. Find seeds in new End-city chests, then plant them. Existing looted chests must remain empty.
9. Run the prior Mending, Frost Walker, End/gateway/city/shulker and box checklists in the same build. Record BUILD_INFO.txt, seed, coordinates and logs for failures.

## Reference assets

Original block/item textures from `InventivetalentDev/minecraft-assets`, ref `1.12.2`, under `assets/minecraft/textures/{blocks,items}`. The End-city seed entry was checked against `assets/minecraft/loot_tables/chests/end_city_treasure.json` in the same ref. Java 1.12.2 magma/beetroot behavior was consulted as a mechanical reference; Xbox-specific comparison remains a play-test task.
