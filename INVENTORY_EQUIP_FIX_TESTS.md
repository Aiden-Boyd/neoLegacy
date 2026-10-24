# Creative Misc and Elytra equipment fixes

Cumulative base: End respawn candidate 56c3c3f1b5cf78a2699cb7c065d2102afd206ebf.

## Confirmed source defects and fixes

The Misc tab starts with a chest and the 16 new shulker-box colors. Their
ResourceLocations hold file paths, with an empty indexed-texture array. The old
TileEntityRenderer::bindTexture unconditionally called getTexture(), indexing
that empty array. It now uses Textures::bindTexture(ResourceLocation*), which
selects indexed or path-based loading correctly. The matching HTTP fallback and
height helpers also select the correct representation. Indexed chest textures
retain their existing behavior. The same fix covers placed shulker boxes and
End-city banners using path-based textures.

ElytraItem::use previously only played a sound. A separate server equipment
hack ran after inventory updates had already been broadcast, producing incorrect
prediction and late armor/hand changes. Elytra now equips through Item::use on
both client and server, before normal synchronization. It copies exactly one item
into the chest slot, preserving damage, name and enchantments, then consumes the
hand stack. Native game modes clear the Survival hand or restore Creative's held
count. Inventory dirty state is set. An occupied chest slot is not replaced, and
failed/tooltip-only attempts do not play equip sounds or change inventory.
The old post-broadcast armor hack explicitly excludes Elytra; other armor keeps
its existing handling.

The physical inventory stores armor in boots-to-helmet order; the menu displays
the reverse order. Existing ArmorSlot and InventoryMenu routes already accept
Elytra in the chest slot. Tests now exercise those real methods, not just the
item's equipment classification. This patch does not change flight physics or
replace player models.

## Automated checks

- `python tests/tile_texture_regression.py`: actual ResourceLocation, native
  Textures::bindTexture dispatch and actual TileEntityRenderer helpers. Checks
  all 16 shulker paths, banner path, indexed chest, HTTP fallback, height lookup,
  null location and unavailable texture context. The fixture asserts if code
  indexes an empty ResourceLocation array, reproducing the original bad path.
- `python tests/equipment_inventory_regression.py`: actual Elytra TestUse/use,
  MultiPlayerGameMode::useItem, ServerPlayerGameMode::useItem, ArmorSlot::mayPlace,
  InventoryMenu::quickMoveStack and AbstractContainerMenu::moveItemStackTo.
  Tests client prediction, tooltip-only calls, Survival/Creative results, metadata,
  separate item instances, occupied chest slots, main-inventory/hotbar quick
  equip, unequip and chest-only placement. Also verifies legacy swap exclusion.
- Existing pagination, flight, furnace, shulker, Overworld and End respawn
  regressions remain enabled, followed by a full Windows64 client build.

These source/fixture results do not establish actual Iggy/GPU/controller behavior.

## In-game acceptance (pending)

1. Launch this candidate through the existing Steam/Proton shortcut.
2. Open Creative Misc, scroll every row, select all shulker colors, leave and
   reopen the tab repeatedly. Test split screen if used. Existing chests should
   retain their icons; placed shulker boxes and End-city banners should render.
3. With an empty chest slot, hold Elytra and press left trigger. Verify it appears
   on the player immediately. Survival consumes the held item; Creative retains
   one. Reopen inventory and confirm the same state remains.
4. Repeat while a chestplate or another Elytra occupies the slot: no replacement,
   duplication or false equip sound should occur.
5. Equip and remove Elytra manually and with quick move from inventory/hotbar.
   Verify it only enters the chest slot. Repeat with damaged, named and enchanted
   wings, then save/reload. Check both local and connected players.
6. Test gliding after equipping. Flight tuning is unchanged. If the inventory
   player preview still looks wrong, capture that separate visual symptom.
