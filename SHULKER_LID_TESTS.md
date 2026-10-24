# Shulker lid collision candidate

Cumulative base: 5c77582ac20a730964f7b482ecff078717b09709.

The lid now displaces entities only when they intersect its extending half-block,
using overlap depth plus 0.01 rather than a fixed 0.05 movement. Both opening
and closing animation clear intersecting entities. Native non-physical and removed
entities are excluded. Dropped items can move too; isPushable is a mob-collision
predicate and incorrectly excluded them. Movement is host-authoritative.
Scratch entity lists and pooled bounds are copied before movement.
Existing six-direction obstruction checks and expanded collision bounds remain.
Named empty boxes now survive Creative breaking, preserving their names.

Cauldron washing is not added: the examined 1.12.2 cauldron implementation has
no shulker-box washing branch. CU59 support has not been established. The
current palette has sixteen dyed boxes and no distinct undyed box, so mapping
washing to purple would silently invent behavior. Dye crafting remains available.

## Validation

The native ShulkerBox.cpp is compiled by tests/shulker_box_regression.py.
Tests cover all six directions, overlap-dependent displacement, entities inside
the base, non-physical/removed entities, closing, client authority and named
Creative drops, alongside prior NBT/lifecycle/clearance cases. The fixture
exercises movement requests, not the full native world collision resolver.
Windows CI also runs prior regressions and builds the full client.

## In-game checks

1. Open/close boxes facing all six directions; verify lift and spin match collision.
2. Stand on upward lids and in front of sideways lids; try dropped items and mobs.
3. Check entities beside/behind the box are unaffected. Test lids near walls and
   ceilings for jitter or trapping; native movement still respects solid blocks.
4. Check full-cube obstruction prevents opening; partial block clearance behaves
   consistently. Two players may keep the same box open; closing one menu should
   not close the lid until the other closes too.
5. Creative-break named empty, unnamed empty and filled boxes. Only the unnamed
   empty box should vanish. Re-place retained boxes and verify name and inventory.
6. Save/reload and repeat through Steam/Proton/controller and with a second player.

Reference: WangTingZheng/mcp940 at d0c030a4139ce7cf3f284b180f0d9ea87bdf8141,
TileEntityShulkerBox.moveCollidedEntities and shouldDrop; BlockCauldron.
In-game GPU, full collision resolution and multiplayer acceptance remain pending.
