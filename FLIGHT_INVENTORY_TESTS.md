# Flight, equipped heads, Creative inventory, and furnace candidate

Based on the End palette candidate at f859f35. This build retains its blocks and
the earlier sponge fixes. The accepted Steam baseline remains unchanged.

## Changes and evidence

- Equipped skulls now use the existing cached SkullTileRenderer, attached to the
  actual player head's posed transform. The previous player-skull armor path
  allocated a SkeletonHeadModel on every armor render pass, replaced a model-part
  pointer, and never freed the replaced objects. Removing that path fixes an
  unbounded allocation and avoids independently animated skull/player heads.
- The skull is scaled to cover the normal head and outer skin layer. Custom
  non-humanoid skins and owner-specific skin lookup still need separate work;
  this does not add a new skin downloading system.
- Holding jump no longer adds Creative-only upward thrust while gliding. The
  existing lift, turning and drag coefficients are retained. Switching to Creative
  flight ends gliding, and Creative vertical controls do not act during gliding.
- Creative pagination now bounds dynamic indices, starts dynamic slots at zero,
  skips empty static/debug groups, stops at exhausted groups and clears entire
  stale stacks. Previously a full dynamic page requested slot 50 in a 50-slot
  selector, and empty groups could be indexed past their contents.
- Furnace quick-transfer now includes item metadata when checking recipes. Wet
  sponge (19:1) can enter the input slot; dry sponge (19:0) cannot. This also fixes
  quick-transfer for other recipes that distinguish metadata, such as salmon.

The reported crash happened after inventory use, not sponge placement. These are
confirmed source defects, but neither a crash log nor a reproducible runtime trace
has linked that particular crash to one of them. Do not mark the crash resolved
until the reproduction is tested.

## CU59 sponge behavior

Wet sponges still dry through furnace smelting. Nether drying is not added to
this CU59 target. Mojang introduced Java Nether drying in snapshot 19w36a on
September 4, 2019, after Xbox One Edition's final content update:
https://www.minecraft.net/en-us/article/minecraft-snapshot-19w36a

## Automated checks

`python tests/flight_inventory_regression.py` compiles actual production methods
with bounded engine fixtures. It exercises full dynamic pages, invalid dynamic
indices, empty intermediate groups, empty debug groups, stale stacks, equivalent
Creative/Survival glide motion with jump held, finite movement at extreme pitch,
and metadata-aware furnace quick-transfer. Both existing regression suites also
run in CI, followed by the full Windows client build.

The fixtures do not establish head rendering, controller feel, real networking,
real furnace execution or complete CU59 physics parity.

## In-game acceptance on Bazzite

Use a separate extraction folder and the same Steam/Proton/controller setup.

1. Equip each of the five head types. Check standing, crouching, inventory preview,
   gliding at different pitch angles, and removing the head again. The skin head
   and outer hat layer must remain inside the worn skull. Check normal helmets too.
2. Wear a player head for several minutes while switching between inventory and
   third-person views; watch for continuing memory growth or another crash.
3. Glide from the same height in Survival and Creative. Holding jump must not
   provide sustained upward thrust. Check double-jump Creative flight still works.
4. Repeat the reported sequence: select a sponge in Creative, move it into the
   hotbar/inventory, switch tabs, scroll, close and reopen the inventory. Check
   both dry and wet variants without placing them, then check ordinary placement.
5. Quick-transfer wet sponge into a furnace with keyboard shift-click and the
   controller equivalent. Add coal and verify a dry sponge result. Quick-transfer
   dry sponge must not route it into the input slot. Check salmon and ordinary ore.
6. Save, restart, and repeat. Recheck the End palette and normal Steam/controller use.

If the crash repeats, record the exact sequence and whether a player head was
equipped. Add `PROTON_LOG=1 %command%` to the Steam shortcut's Launch Options,
reproduce once, and collect the resulting `steam-*.log` from your home folder.
Remove the option afterward to stop generating extra logs.
