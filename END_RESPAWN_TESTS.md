# End Crystal and dragon respawn candidate

Cumulative base: `feature/overworld-integration`, 6f16b91703056dd2bf2cb8edd5deeff4aced06c3.
Target: Xbox One CU59, preserving the native LCE eight-pillar arena. Java 1.12.2
is the reference for crystal placement, ritual stages and repeat-kill rewards.
This is a candidate for in-game acceptance, not a full CU59 parity declaration.

## Implemented behavior

- End Crystal item 426, original icon in previously unused item-atlas cell (13,7),
  localized name/use hint, Creative entry and enchanted glint. The 3x3 recipe is
  seven glass, an Eye of Ender in the center, and a ghast tear at bottom center.
  Both native recipe registration and its vararg parser are exercised by tests.
- Placement on obsidian/bedrock requires permission, two replaceable blocks of
  clearance and an unoccupied 1x2 column. Tooltip preview/client prediction do
  not spawn/consume anything. The server consumes one item only after addEntity
  succeeds; Creative inventory is preserved. Placed crystals hide the model base.
- Synchronized beam target and base visibility use ordinary entity metadata (IDs
  9–13). BeamTarget, ShowBottom and the existing Entity Invulnerable field persist.
  Natural old crystals default to visible bases and no beam. Rendering uses the
  native dragon beam texture and restrained animation rate. Crystals only create
  fire in End air; they cannot replace a bedrock cap or other solid block.
  Explosion damage no longer recursively generates another crystal explosion.
- Four distinct crystals around the central exit fountain start the ritual only
  after a dragon has been defeated and when no dragon remains. Place them on the
  four cardinal bedrock rim blocks, three blocks from the fountain center. Their
  existing two-block entity bounds intersect the vanilla trigger volumes.
- The exit closes, four beams point upward, preparation runs for 100 ticks,
  pillars regenerate one per 40 ticks, then a 100-tick summoning stage creates
  one dragon. The four ritual crystals explode without block destruction and
  disappear. Pillar crystals become destructible and lose their ritual beams.
  Native pillar geometry/cages are retained, with original heights captured from
  existing caps or obsidian tops (including saves affected by the old fire bug).
  Regeneration intentionally clears the 21x21x21 area around each pillar top.
- Host-owned state stores phase/timer, first-kill/defeated status, dragon UUID,
  four crystal UUIDs, fountain height and all eight pillar heights. LevelData
  forwards it through DerivedLevelData and copies it when worlds are copied.
  No process-global pointer cache or transient entity ID is used for persistence.
- Progress pauses when the central arena is unloaded or all players leave it.
  Destroying a ritual crystal aborts, clears survivor beams/pillar invulnerability
  and restores the active exit. Missing/duplicate saved crystal IDs also abort
  after the arena has loaded. Failed dragon creation retries without consuming
  crystals. A dragon already loaded at the final spawn boundary is adopted,
  preventing a second spawn. The arena must be loaded; chunk tickets/forced
  arena loading are not introduced in this candidate.
- First dragon death pays 12,000 XP and places one egg. Repeat deaths pay 500 XP
  without a new egg. XP obeys doMobLoot. Death-animation progress and reward
  budget save with the dragon; a completed saved death cannot re-award XP, egg
  or gateway. Gateway sequencing still uses the previous saved 20-gateway limit.
- Old saves migrate using the loaded active central exit and saved gateway count.
  No ritual starts simply because a dragon is missing from a newly loaded chunk.
  The manager does not introduce automatic initial-dragon replacement or a full
  Java fight-manager rewrite.

## Automated validation

`python tests/end_fight_regression.py` compiles the production ritual/state,
EnderCrystal, EndCrystalItem, native SpikeFeature and dragon death/save methods
against an engine fixture. It covers:

- Placement support/permission/clearance/occupancy, preview/client, Creative,
  unloaded cells, failed spawn, default metadata, fire restrictions and explosions.
- Crystal metadata save/load, legacy defaults and invulnerability persistence.
- Trigger gates, unique UUIDs, full ritual with save/reload every seven ticks,
  all eight actual pillar shapes/cages, four final explosions, one dragon and
  eight destructible normal crystals.
- Every abort phase (including final boundary), missing crystals on reload,
  unload/player departure pauses, failed spawn retry, existing-dragon recovery,
  old missing-cap recovery and duplicate saved IDs.
- Exact first/repeat XP totals, doMobLoot, egg/gateway counts, death-animation
  reload and completed-death replay prevention.

`python tests/end_crystal_integration.py` compiles the real recipe registration/
parser and validates item, UI, atlas, host tick, world-save forwarding and renderer
integration. Existing asset preservation and cumulative regressions remain enabled.
The full Windows64 client must compile and package before this is downloadable.
Fixtures do not prove GPU appearance, real chunk/entity loading or controller UI.

## In-game acceptance (pending)

1. Use your existing Steam/Proton/controller setup. Check the Creative End Crystal
   entry, name, icon/glint, use hint and classic/console recipe. Test all supports,
   blocked placement, entities in the column and Creative/Survival consumption.
2. Place a crystal in the Overworld/Nether: no fire should appear. In the End,
   verify an occupied solid cell is preserved. Attack a crystal and check explosion.
3. Complete a first dragon kill: 12,000 XP, active fountain, one egg and gateway.
   Existing defeated worlds should proceed directly to the next step.
4. Place four crystals on the cardinal bedrock rim blocks (center +/−3 x or z),
   with no other entities obstructing placement. Observe portal closure, beams,
   staged eight-pillar rebuilding, cages, summoning and four final explosions.
5. Destroy a ritual crystal during preparation, pillar rebuilding and summoning.
   Verify cancellation, active exit, cleared beams and vulnerable pillar crystals.
6. Save/reload mid-preparation, mid-pillar, near completion and during the dragon
   death animation. Verify resumed progress, one dragon and no repeated rewards.
7. Leave the End/central arena mid-ritual, return, and test a second connected
   player. Check metadata, pillar updates, UI, damage and portal synchronization.
8. Kill the respawned dragon: 500 XP, next gateway, no new egg. Repeat with
   doMobLoot=false. Confirm a normal unrelated crystal does not trigger an abort.

Ritual audio currently uses the existing native dragon growl. Exact CU59 sound,
particle, beam timing and controller feel still need comparison in-game. Current
LCE combat phases are retained; this change does not replace dragon combat AI.

## Primary references

Read from `WangTingZheng/mcp940`, commit d0c030a4139ce7cf3f284b180f0d9ea87bdf8141:
`world/end/DragonFightManager.java`, `world/end/DragonSpawnManager.java`,
`item/ItemEndCrystal.java`, `entity/item/EntityEnderCrystal.java`, and
`world/gen/feature/WorldGenSpikes.java`. Arena compatibility follows this port's
`TheEndBiomeDecorator::SpikeValA` and `SpikeFeature::placeWithIndex`.
Original icon: `InventivetalentDev/minecraft-assets`, ref `1.12.2`,
`assets/minecraft/textures/items/end_crystal.png`.
