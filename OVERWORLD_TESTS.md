# Overworld integration candidate

Cumulative base: `feature/nether-beetroot` at c1c9f8f23585038cf37e6ee658008406459981f4.
Target remains Xbox One CU59. Java 1.12.2 is a behavior reference, not a claim that
this candidate implements every Java or CU59 feature.

## Implemented

- Grass paths, block 208: shovel use on uncovered grass (any face except bottom),
  permissions and tooltip-preview checks, one durability on a successful server
  write, original top/side textures and all atlas mips, Creative entry and name.
  Shape and collision are 15/16 high. Solid cover converts the path to dirt;
  ordinary and Silk Touch harvest drop dirt. Paths on grass village roads retain
  the existing desert road palette and road base behavior.
- Both village farm sizes select carrots 20%, potatoes 20%, beetroot 10%, wheat
  50%. Beetroot ages are 1–3; other crops stay 2–7. Saved crop IDs remain intact.
  Existing generated villages are not retroactively rebuilt.
- Adult farmers collect nearby wheat/seeds, bread, carrots, potatoes and beetroot/
  seeds into eight real ItemInstance slots. Tags/metadata and slot contents save
  under `Inventory`. Partial pickup leaves the exact remainder in the world;
  full inventory rejects excess. Farmers harvest mature crops and replant empty
  farmland using existing seeds/roots, consuming one only after a successful write.
  Server-only actions obey mobGriefing and pause during trading. Navigation is
  bounded to 16 blocks, checks loaded chunks, requires physical proximity, and
  times out. They stop harvesting once sufficiently stocked with food.
- Native farmer trade pool gains carrot and potato purchases (15–19). No beetroot
  purchase is added: Java 1.12.2 farmers do not buy beetroot. Existing LCE offers
  and saved trades remain valid. This is not a complete villager-career rewrite;
  food sharing, breeding from food and wheat-to-bread conversion remain separate.
- Desert and swamp population can place fossils (1/64 chance per decorated chunk).
  Eight bone structures and eight matching coal overlays come from original 1.12.2
  NBT. Sparse cells preserve intervening terrain, bone axes rotate, integrity is
  90% bone/10% coal. Placement remains inside one loaded chunk, at least y=10,
  below the minimum terrain height across the rotated footprint. Bedrock, liquids,
  tile entities and non-natural blocks are protected. The separate seeded stream
  preserves other decoration draws. Layouts are deterministic for this port;
  exact Java seed-to-fossil coordinates are not promised. Old populated chunks
  are not scanned or modified. See `assets/fossils/provenance.json` for hashes.

## Automated checks

`python tools/generate_fossil_header.py --check` validates every original template,
its sparse positions/palette, generated native arrays and provenance.

`python tests/overworld_regression.py` compiles production path, shovel use, farmer
inventory/pickup/goal and fossil code against an engine fixture. It tests authority,
permissions, preview, failed writes, collision, inventory capacity/tags/save reload,
exact-once pickup, all crop/seed types, immature crops, disabled griefing, baby/
profession/trading gates, unreachable paths, and every fossil/rotation including
negative coordinates, deterministic output, integrity, unloaded/protected cells.
The fixture does not substitute for native pathfinding or an in-game GPU test.
Existing cumulative regressions and full Windows64 build remain required.

## In-game acceptance (pending)

1. Launch from Steam using the existing Proton setup and controller profile.
2. Create paths with each shovel; verify tooltip, durability/Creative mode,
   footsteps, 15/16 collision, side culling, solid-cover conversion and dirt drops.
   Audio currently uses the existing grass step sound, not the dedicated Java
   shovel-flatten sample. Check split-screen/remote-client block synchronization.
3. Generate new normal/desert villages. Find beetroot rows, check ages/textures,
   harvest/replant, and verify desert paths retain their palette.
4. In daylight, give an adult farmer wheat seeds, beetroot seeds, carrots/potatoes;
   watch physical harvesting and replanting. Save/reload and repeat. With
   mobGriefing=false verify pickup and field changes stop. Test inventory overflow
   and a remote client; confirm no free seeds or duplicate drops.
5. Find underground desert/swamp fossils in newly generated chunks; inspect bone
   axes and coal patches and save/reload. No preexisting chunk retrofit is expected.

## Primary behavior references

Read from `WangTingZheng/mcp940` at d0c030a4139ce7cf3f284b180f0d9ea87bdf8141:
`BlockGrassPath`, `ItemSpade`, `StructureVillagePieces`, `EntityAIHarvestFarmland`,
`EntityVillager`, `WorldGenFossils`. Original texture/structure assets are from
`InventivetalentDev/minecraft-assets` ref `1.12.2`. No broad CU59 parity sign-off
has been made; see `END_FIGHT_AUDIT.md` for the next End work.
