# Rockets, World of Color and observers

Cumulative base: Creative/Elytra fix 05e18bb9de9677da79575279d82b3dd4b5d4aac1.
Target remains Xbox One CU59, with Java 1.12.2 behavior references. In-game
Steam/Proton/controller, GPU and multiplayer acceptance is pending.

## Features

- Using a firework while gliding attaches a rocket to the player. Each tick uses
  the reference acceleration toward the look vector. Flight 1–3 gives 20–31,
  30–41 or 40–51 ticks respectively. No acceleration when gliding stops. Normal
  ground launches remain available. Host-only spawn, client consumption prediction,
  native Creative restoration, failed-spawn retention, copied one-item NBT,
  attachment metadata and UUID save/load are included. Missing owners expire
  safely rather than attaching to a different entity. The old lerpMotion parameter
  shadowing bug is fixed as part of network motion support.
- Plain rockets cause no explosion damage. Each explosive star contributes to
  the reference damage budget (5 + 2 per star); the glider takes the full amount,
  nearby living entities take distance-scaled damage within five blocks with
  line-of-sight checks. Uses an explosion-typed firework damage source and localized
  death messages. Existing star visuals/audio remain intact.
- Concrete 251 and concrete powder 252 have all 16 color variants, distinct
  names, icons, synchronized metadata, correct drops and Creative entries.
  Four normal sand + four gravel + one matching dye crafts eight powder.
  Powder falls, preserves color in animated and instant fall paths, hardens when
  touching water from above or a side, and checks every crossed voxel during fast
  falls through water. Water only below a stationary block does not immediately
  harden it. Animated landing in liquid follows the 1.12 landing rule.
- Glazed terracotta 235–250: all 16 colors, smelt matching stained clay for 0.1 XP,
  four horizontal placement orientations, reference face-pattern rotations and
  direction-independent drops. Ordinary pistons can push them; sticky pistons
  cannot pull them. Full slime-block contraption parity depends on the existing
  native piston implementation and is not established by this batch.
- Observer 218: six placement facings, watched front and output back, powered
  texture, strength 15, two-tick delay and two-tick pulse, deduplicated scheduling,
  placement pulse and output updates when removed while powered. Successful
  Level block and metadata writes notify observers; neighbor power notifications,
  unchanged writes and forced no-op updates do not. Hidden gameplay metadata writes (including dispenser triggers) are observed.
  Bulk no-light generation writes (flag 12) and explicit suppression bit 16 do not start pulses. Native scheduled-tick
  persistence is retained; no tile entity or new save format is required.
- Original 1.12.2 textures are inserted into 104 previously empty, unregistered
  atlas cells. Multi-cell water/lava allocations are reserved. Existing pixels
  and mip cells are preserved. Rotated glazed/top images are derived losslessly;
  the native renderer's shared UV state is unchanged. Texture hashes/provenance
  are in tests/color_atlas_manifest.json. Manual regeneration uses
  tools/generate_color_atlas.py with Pillow; CI asset verification needs no Pillow.

The native map renderer still has its old limited color palette. Full map-color
parity and custom texture-pack fallback are separate work, not certified here.

## Checks

- tests/color_observer_regression.py compiles production blocks, observer and
  actual Level block/metadata write methods: all colors, water sides, no client
  writes, color-preserving fall paths, fast-water traversal, failed writes,
  glazed placement and texture choices, every observer facing, no-op/suppressed
  writes, pulse coalescing, directional output, removal and saved queued ticks.
- tests/rocket_boost_regression.py compiles production rocket logic and item-use
  methods: authority, Flight lifetimes, reference acceleration, stop-gliding,
  source-stack isolation, normal launches, save/reload, missing owners, attached
  client metadata, star damage, distance and wall protection.
- tests/color_recipe_regression.py compiles actual registrations and native
  vararg parsers/shapeless matcher: observer layout, 16 powder recipes, counts,
  dye metadata, rejected red sand/missing ingredients, furnace and Creative wiring.
- tests/color_assets_regression.py verifies original hashes, all atlas cells/mips,
  quarter-turn pattern pixels and preservation of all previous pixels.
- All earlier cumulative regressions and the full Windows64 client remain required.

## In-game acceptance

1. Launch through Steam with the current Proton/controller profile. Check every
   new Creative entry/name/icon, especially last pages and all 16 colors.
2. Glide and use plain rockets of Flight 1–3. Test turning/climbing, landing while
   attached, repeated use, Creative retention, Survival consumption and a second
   player watching. Save/reload while boosting and change dimensions/disconnect.
3. Repeat in Survival with starred rockets: glider damage, nearby damage, wall
   shielding, explosion visuals/audio and death messages. Plain rockets must not hurt.
4. Craft all powder colors; test water above/side/below, falling through a thin
   water layer, fast falls, color retention, landing, breaking and save/reload.
5. Smelt stained clay, place glazed blocks from four directions and make 2x2
   patterns. Test normal piston pushes and sticky-piston retraction.
6. Place observers in all six directions. Change front blocks, crop ages, lever
   states and fluid states; check a short pulse only at the back. Change side/back
   blocks, repeat unchanged states, remove powered observers, move with pistons,
   and save/reload with a pending pulse. Test observers watching other observers.
7. Recheck Misc/shulker rendering, Elytra hand/menu equip and the dragon ritual.

References read: WangTingZheng/mcp940 commit d0c030a4139ce7cf3f284b180f0d9ea87bdf8141,
BlockObserver, BlockConcretePowder, BlockGlazedTerracotta, ItemFirework and
EntityFireworkRocket. Assets/models: InventivetalentDev/minecraft-assets ref 1.12.2.
