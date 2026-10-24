# End recovery and chunk-safe respawning

Base: 6c560d4ca2e015467b29da13072edcc8443aff20. Cumulative candidate.

While a player is within 192 blocks of the center on both horizontal axes, the
host requests missing central chunks (-8..7 on each chunk axis) and the tracked
active dragon's last chunk plus its immediate neighbors. At most four missing
chunks are requested per tick. Ritual ticks and absence observation wait until
all requested chunks are present, with at least one tick after loading requests.
The native chunk unload loop retains these chunks only while players are nearby;
its queue rotates pinned entries so other chunks may still unload. When everyone
leaves, normal unloading resumes. The central area can retain 256 chunks, plus
up to nine tracked-position chunks: memory/load time must be evaluated in-game.

Tracked dragon UUID and last chunk are persisted. A missing undefeated dragon
is replaced only after 1200 consecutive eligible loaded ticks (60 seconds at
20 TPS). Missing chunks, leaving the center and reloading restart that observation
window. Spawn rejection retains the old identity and retries after another
window. Loaded dragons, including death animations, are adopted rather than
replaced. Recovery does not change first/repeat reward history. Completed worlds
remain completed, including legacy migration with an existing gateway but a
missing portal. Regular four-crystal respawning remains required after a kill.

An older save with a tracked UUID but no position is handled conservatively:
automatic replacement is disabled until that dragon is found and its position
recorded. This avoids replacing an existing dragon hidden in an unknown unloaded
chunk. Recovery of a dragon already missing from such an old save is not solved
by guessing. Worlds with no prior UUID can recover an absent initial dragon
once the complete central search area has loaded and the delay has elapsed.

Destroying a ritual crystal while arena chunks are absent persists PendingAbort.
Portal restoration waits for chunk readiness. Ritual phases/ticks, pillar
heights and crystal IDs continue to use the existing persisted record. Leaving
pauses the ritual; returning reloads required chunks before checking crystal
presence. Save/reload is not an atomic database transaction: this batch does not
promise recovery from a power loss midway through a disk save or pillar write.

## Validation

The native EndFight implementation, EndFightState serialization, dragon death
methods, pillar generation and ServerChunkCache::tick are compiled in the
regression fixture. Expanded cases cover bounded requests, queue pin/release,
known-position adoption, delayed initial recovery, failed spawns, old UUIDs with
unknown position, completed worlds, missing chunks and no-player reset, deferred
abort save/load and interruptions in every ritual phase. Existing tests reload
through the whole ritual, verify eight pillars and four consumed crystals, and
check single-dragon spawning, 12000/500 XP, first-only eggs and gateway counts.
Full Windows compilation and cumulative tests remain required.

## In-game acceptance

1. Enter an existing End fight and leave/re-enter the central island. Verify the
   same dragon remains and the frame rate/memory footprint is acceptable.
2. In a disposable test save, record the active dragon under this build, remove
   it with a development tool, then stay near the island for 60 loaded seconds.
   Verify exactly one replacement and no extra eggs/XP/gateway history changes.
3. Start four-crystal respawning; leave, reload and return at each ritual stage.
   It should resume after chunks load, preserve crystals while paused and produce
   one dragon. Destroy a ritual crystal during an interrupted stage; the exit
   should return once the arena loads.
4. Repeat kills and verify first-only egg, reduced repeat XP and one gateway per
   defeat. Test death-animation save/reload and doMobLoot=false.
5. Use multiplayer: one player stays central while another travels outward,
   then both leave. Check unrelated chunk unloading continues and returning does
   not create duplicate dragons/crystals. Check Steam/Proton/controller as usual.

Fixtures do not reproduce asynchronous native disk I/O, real player chunk maps,
GPU rendering or full multiplayer. These acceptance checks remain pending.
