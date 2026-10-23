# End update candidate 1: building palette

Branch: `feature/end-palette`, based on `fix/base-stability` at 7489a14.
Target: Xbox One Edition CU59 End content and behavior, built for Windows64 and
tested through Steam/Proton on Bazzite. This is the first End milestone, not the
complete expanded End. The accepted `steam-proton-base` remains the fallback.

## Included

- Purpur Block (201), Pillar (202), Stairs (203), double slab (204), slab (205),
  and End Stone Bricks (206), using the previously reserved IDs.
- Five visible items in Creative's Building Blocks tab, immediately after End Stone.
  Double slabs are formed by placing two slabs together, not listed separately.
- Pillar orientation, top/bottom slab placement, slab merging and normal drops
  use the existing pillar, stair, slab and block-item engine paths.
- Four recipes in the existing console/classic registry: four End Stone to four
  bricks; six purpur blocks to four stairs; three purpur blocks to six slabs;
  two vertically arranged slabs to two pillars.
- Original 16x16 textures packed into default atlas row 25, columns 0-3. Both
  supplied mip levels are updated. Existing atlas cells are preserved.
- English item names and hints through the existing localization XML pipeline.

Purpur is available through Creative in this milestone. Chorus fruit and the
popped-fruit-to-purpur recipe arrive with the chorus milestone; no substitute
Survival recipe is introduced. End Stone Bricks can be crafted in Survival now.

## Automated verification

Run `python tests/base_stability_regression.py` and
`python tests/end_palette_regression.py` with g++/clang++ or in an MSVC developer
shell. The End test compiles the actual new tile classes and the actual shared
pillar/slab implementations with an engine fixture. It checks all six pillar
placement faces and resulting textures, orientation-free drops, slab placement
and shapes, and one/two-slab drop counts. CI also builds the complete client.
These checks do not simulate network transport, saving, rendering or crafting UI.

## In-game acceptance

Extract to a separate folder and use the same Steam/Proton/controller setup as
the working base. Use the default texture pack and a disposable world copy.

1. Find all five items after End Stone in Creative Building Blocks. Check names,
   inventory icons, placed textures and tooltips. Check nearby old blocks too.
2. Place pillars on all six faces; verify end grain and stripe direction. Break
   differently oriented pillars in Survival with a pickaxe: drops must stack.
3. Place stairs in each direction, upside down and in inner/outer corners.
4. Place bottom and top slabs, walk over them and combine matching halves.
   A double slab must drop two slabs; a half slab must drop one. Check Silk Touch
   and pick-block behavior as well. Purpur must not merge with sandstone/stone.
5. Test all four recipes in both classic and console crafting, including ingredient
   consumption and result counts. Pillars use two slabs, not two full blocks.
6. Save, fully exit and reload. Check IDs, pillar orientation, stairs and slabs.
7. Check close and distant textures with mipmapping enabled. Optional DLC/custom
   packs have not been extended with these atlas cells in this milestone.
8. Repeat placement and breaking with a second client if available; check both
   players see matching block data and inventory results.
9. Recheck Steam launch, controller navigation, saving and clean exit, plus the
   sponge checks in BASE_STABILITY_TESTS.md.

Do not merge into the accepted baseline until these checks pass.

## Remaining End work

The existing End still uses the earlier main island generator and boss behavior.
The next milestones are:

1. Chorus plant/flower growth and drops, fruit teleportation, popped fruit,
   purpur crafting, and end rods with complete placement/rendering.
2. Expanded End bounds and storage, deterministic outer islands, gateways and
   safe return travel; preserve the original central island and existing saves.
3. End cities and ships with loot, obtainable Elytra, dragon heads and shulkers
   including their bullets, attachment, teleporting and levitation behavior.
4. Dragon fight state, respawning with crystals, gateway progression and the
   remaining CU59-specific behavior, followed by controller/Steam acceptance.

## Texture provenance

Source: original Java 1.12.2 client textures, retrieved from the minecraft-assets
mirror at `InventivetalentDev/minecraft-assets`, ref `1.12.2`, under
`assets/minecraft/textures/blocks/`. These are Mojang game assets, not newly
licensed open-source artwork. No decompiled Java implementation was copied.
Files: purpur_block.png, purpur_pillar.png, purpur_pillar_top.png, end_bricks.png.
The individual images remain alongside the atlas under the default TitleUpdate
resource directory for inspection. The target is the original console palette;
visual parity still requires comparison in-game.
