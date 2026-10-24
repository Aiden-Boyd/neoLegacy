# End fight audit

Status: the findings below describe the Overworld baseline. The subsequent
`feature/end-respawn` candidate implements crystal placement, synchronized/saved
beams and base visibility, a saved staged ritual, interruption recovery, and
first/repeat XP/egg rules. See `END_RESPAWN_TESTS.md` for implemented scope and
pending in-game acceptance. Initial-dragon recovery, exact CU59 visual/audio
parity and full combat-AI comparison remain separate work.

## Present

- `EnderDragon.cpp`: native LCE flight/path nodes, strafing, perching/scanning/
  attacks, healing from crystals, crystal-destruction handling, death animation.
- `EnderCrystal.cpp`: saved entity registration, destructible crystals, explosion,
  and notification of the living dragon. Gameplay synchronization exists for
  remaining life; this is not a modern beam-target protocol.
- `EnderDragon::tickDeath`: death XP, exit fountain creation, and cumulative
  `EndGateways::dragonDefeated(level)` call. Gateway sequencing/persistence and
  outer End transport are already separate implemented features.

## Original baseline gaps (addressed by the respawn candidate unless noted)

1. No player-usable End Crystal item, placement checks, recipe or Creative entry.
   The existing crystal entity alone cannot initiate the four-crystal ritual.
2. No saved fight manager or respawn phase machine that detects four crystals
   around the fountain, closes the portal, targets beams, regenerates pillars/
   crystals, spawns exactly one dragon, and finishes/aborts safely.
3. `EnderCrystal::addAdditonalSaveData` and `readAdditionalSaveData` are empty.
   No saved beam target or ShowBottom field. `tick` creates fire regardless of
   dimension; a placeable item needs End-only fire behavior and replaceability
   checks so crystals cannot erase blocks in other dimensions.
4. `tickDeath` awards the legacy 12,000 XP sequence on every death. Java 1.12.2
   distinguishes first kill (12,000) and repeat kill (500). `spawnExitPortal`
   unconditionally places an egg; repeat kills must not create new eggs.
5. Existing gateway-count persistence does not establish first-kill state, ritual
   progress, active crystal UUIDs or active dragon identity. Loading during the
   ritual and migration of old defeated worlds need explicit handling.
6. Destroying a ritual crystal, chunk unload/reload, dimension changes, duplicate
   crystal placement and multiple clients need host-authoritative, idempotent
   behavior. Neither a successful build nor a mock entity test establishes this.

## Implementation sequence used

1. Add End Crystal item/recipe/UI and safe placement; extend crystal data/save/
   packet/renderer support for beams and bottom visibility. Keep existing natural
   crystals compatible with old saves.
2. Add persisted fight state on the host: first-kill flag, dragon UUID, ritual
   phase/timer and crystal IDs, with conservative migration and recovery rules.
3. Implement four-crystal trigger and staged pillar regeneration/dragon creation,
   including interruption cleanup and successful completion exactly once.
4. Route fountain/egg/XP decisions through fight state; retain the existing
   gateway sequence. Add phase/save/load/abort/duplicate regression fixtures and
   full native build checks.
5. Test a complete first kill, repeat ritual, interrupted ritual and mid-ritual
   save/load from Steam with controller and a connected client.

Reference baseline for behavior: Java 1.12.2 `DragonFightManager`,
`DragonSpawnManager`, `ItemEndCrystal`, `EntityEnderCrystal`, `EntityDragon`.
CU59-specific acceptance still requires comparison with Xbox One behavior.
