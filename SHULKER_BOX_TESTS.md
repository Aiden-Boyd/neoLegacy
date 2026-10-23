# Shulker box candidate

Branch `feature/shulker-boxes`, based on the accepted-compilation End city and
shulker candidate. The previous build remains separately downloadable. This is
a Windows64 build for testing through the existing Steam/Proton setup on Bazzite.
In-game/controller acceptance is still required; compilation and fixtures alone
do not establish vanilla CU59 parity.

## Added

- All 16 colored box blocks/items (IDs 219–234); 27 storage slots each.
- Item contents, nested item NBT and custom names survive world save/load,
  break/pickup and replacement. Items are unstackable. Tooltips preview up to five
  stored stacks and count additional stacks.
- Facing follows the clicked face. Animated lids open by half a block, with
  original box textures/open-close sounds, clearance checks and expanding
  collision bounds. Boxes never merge into double chests.
- Host and client slot rules reject other shulker boxes. Shift-click and direct
  controller slot swaps enforce that restriction; hopper insertion uses the
  container's native `canPlaceItem` check. Other items can enter/leave normally.
- Creative exposes all colors. Empty Creative breaks do not drop a box; filled
  Creative breaks preserve contents in one dropped box. Server-side removal
  serializes once rather than spilling or duplicating its contents.
- Craft the purple box with a vertical shell / chest / shell recipe, registered
  through the existing console/classic recipe registry.
- Piston reaction is destruction/drop, not moving a live tile entity. Comparator
  fullness is exposed through the normal container helper.
- Corrected the missing comma before the first shulker sound family, which had
  concatenated two names and shifted later sound mappings.

## Remaining work

Dye recipes are deliberately not registered yet: the console crafting path must
preserve filled-box NBT before recoloring is enabled. Colored boxes other than
purple currently come from Creative. Dispenser auto-placement, cauldron washing,
precise lid entity-push parity and all dedicated box sound/particle variations
remain to audit. Breaking particles currently reuse purpur. This batch does not
add Mending or certify the earlier End features as in-game accepted.

## Automated verification

Run `python tests/shulker_box_regression.py`. It compiles production box code
against a focused engine fixture and checks all colors through save/break/place,
deep-copy isolation, retained custom NBT/names, repeated removal, nested-box and
invalid-save rejection, Creative/client drop guards, six orientations, blocked
opening and multiple viewers. Extracted production slot/quick-move methods are
checked for rejected nested placement and allowed ordinary transfer. A separate
compiled check validates the complete sound-name table, including the shulker
and box indexes. Existing content/regression checks and the full Windows build
remain in the candidate workflow. These fixtures do not simulate actual network,
GPU, console crafting or hopper ticks.

## In-game acceptance

Use a copied or disposable world. Both multiplayer players must run this build.

1. Craft a purple box in both crafting interfaces. In Creative find all colors;
   verify names, inventory/held/dropped appearance and placed textures.
2. Place on floors, walls and ceilings. Open with a controller; check lid movement,
   opening obstruction, close sound, collisions and adjacent boxes staying separate.
3. Fill slots 0 and 26, including enchanted/damaged/renamed items. Rename the box.
   Save/reload, break it in Survival, collect the one drop and place it again.
   Contents, item tags and custom name must match. Check the carried-item preview.
4. Try normal placement, shift-click, controller swaps and hopper insertion of
   every box color into another box. They must fail without losing the source
   item. Ordinary items should transfer; hoppers should extract normally.
5. Break empty and filled boxes in Creative. Exercise piston breaking, explosions
   and comparator output in the disposable world; record differences from CU59.
6. Have two players open the same box, move items, close in different orders,
   disconnect and reconnect. Verify no stuck lid, inventory divergence or dupes.
7. Recheck the earlier shulker, city, gateway, sponge and controller behavior.
   Include seed, coordinates, `BUILD_INFO.txt` and logs when reporting failures.

## Asset provenance

The 16 shulker color textures and two box OGG files are reference 1.12.2 assets
retrieved from `InventivetalentDev/minecraft-assets`, ref `1.12.2`:
`assets/minecraft/textures/entity/shulker/shulker_<color>.png` and
`assets/minecraft/sounds/block/shulker_box/{open,close}.ogg`. They are used by this
personal game build; this document makes no redistribution-license claim.
