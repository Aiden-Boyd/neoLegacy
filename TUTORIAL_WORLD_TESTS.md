# TU46 tutorial import compatibility candidate

Cumulative base: e1938be8216509b10393da2fa838308572de02fb.

The supplied converted map exposed unregistered slime blocks (165), standing
banners (176) and the canonical Banner tile-entity identifier. This batch adds
those registrations, wall/standing banner placement, data persistence and packet
sync, plus base-color and pattern rendering. The older EndCityBanner identifier
still loads; new saves use Banner. Older city banners without Base retain the
previous green/black-triangle appearance. All 16 base colors are in Creative.
Banner drops preserve name, color and pattern order, including unknown pattern
IDs (unknown patterns are stored but not rendered). Rendering limits overlays to
20 layers. Inventory previews show base color, without the pattern overlays.

Slime has a translucent original texture, .8 friction, landing bounce, suppressed
fall damage while not sneaking and grounded movement slowing. Living entities
bounce with their downward speed; other entities use .8 of that speed. Sneaking
suppresses bounce and slowing and restores normal fall damage. Drag runs every
grounded move rather than relying on the old sound-distance-gated stepOn hook.
Recipes add nine slimeballs <-> one slime block and all 16 plain wool banners.
Pattern crafting/duplication/washing and sticky slime piston assemblies are not
implemented by this import-focused batch. Do not assume contraption parity.

Textures are the original 1.12.2 assets from InventivetalentDev/minecraft-assets.
The flat item icon derives from the original banner cloth with a drawn pole.
Slime uses terrain cell (31,15); banner icon uses items cell (13,10). All existing
atlas pixels outside those cells were compared with the cumulative base. Blank
added cells in older packs fall back to defaults; supplied artwork is retained.

## Automated checks

`python tests/tutorial_compat_regression.py` compiles production banner data,
entity, item and slime helpers, plus Entity::checkFallDamage and the actual
recipe registration/parser. It covers colors, ordered/unknown pattern layers,
legacy defaults, standard item NBT without Base, clone independence, packet
payloads, survival drops, repeated removal, Creative suppression, all standing
rotations/wall faces, missing support, preview/client guards, denied placement,
failed writes, bounce/no-bounce and fall damage. Asset hashes and unchanged atlas
regions are verified. `tests/map_texture_regression.py` exercises the production
terrain and banner-icon fallback at multiple resolutions/mips. CI also compiles
the complete Windows64 client and runs all existing cumulative regressions.

These checks do not establish GPU output, actual map loading, multiplayer or
Steam/controller acceptance. The private tutorial save is not included in git.

## In-game tests

1. Keep the original archive and saves. Use a disposable copy of the converted
   TU46_Tutorial_World folder with this candidate. Spawn should be near
   (-15,71,85). Explore, save, quit, reload and check all three dimensions.
2. Check the tutorial's white/red standing and wall banners. Inspect both sides
   and several viewing angles. Break/place in Survival, remove the supporting
   block, and ensure exactly one correct banner drops. Creative should drop none.
3. Place all 16 banner colors upright and on each wall face, save/reload, then
   have a second player join. Check colors, names and any existing pattern layers.
4. Drop onto slime from a safe test tower in Survival; check bounce and no damage.
   Repeat while sneaking: no bounce and normal damage. Walk across it, throw an
   item onto it and compare to ordinary blocks. Check translucent faces and edges.
5. Craft and unpack slime; craft plain banners with matching wool colors. Check
   both classic and console recipe menus. Test the Creative Misc tab/controller.
6. Reload an older texture pack and one with supplied artwork in the new cells.
   Check default fallback, preserved custom artwork and distance mipmaps.

This imports the map. The TU46 tutorial lesson/hint progression is not connected
to the older engine scripts. Existing dragon data is preserved by conversion;
its compatibility still needs a real load/play/save test.

## Activated tutorial lessons

The lessons candidate builds on baba9e9e9b70ec8a29ee5ec222d3564447b1aa3b.
Use the separate TU46_Tutorial_World_with_lessons package. The activation tool
opts its existing MapOptions into native FullTutorialMode with dataTag 0x54553436.
Unmarked saves keep their normal behavior. The marker survives native game-rule
serialization through the existing base dataTag attribute. No save-name or seed
heuristic is used. All 12 region files, level.dat and every other virtual file
except requiredGameRules.grf remain byte-identical to the converted map.

The source supplies 23 named areas. The existing introductory sequence and older
area lessons use those rules. Three short area hints introduce banners, slime
and Elytra. These hints use the correct local player's position, require the
Overworld, and appear once per session. The base hint destructor is now virtual,
so retiring area hints releases their bounds. The rule loader also excludes the
already-consumed four-byte string-table length from the remaining payload size.

Activation preserves the original rule content except MapOptions.dataTag, and
re-encodes its outer envelope with platform-independent RLE. It does not rebuild
the terrain. The tool refuses unsupported layouts and an existing output path.
Xbox rule decoding requires the separate MIT LCEStudio codec.py in --codec-dir;
that helper and license accompany the private map package, not the repository.

`python tests/tutorial_lessons_regression.py` compiles the production AreaHint
and marker predicate and tests local-player selection, missing players, both
other dimensions, one-shot behavior, state gating and polymorphic cleanup. It
also exercises the actual activation tool with synthetic rules/save entries,
checking deterministic output, idempotence, preserved files and malformed input.
The supplied map was separately activated and all untouched VFS bytes compared.

Playtest the new package: confirm the initial overview, movement, wood gathering,
crafting and furnace steps. Try skipping and completing the basic sequence.
Visit the brewing, farming, redstone, trading and other existing lesson areas;
then visit the banner/slime/Elytra displays. Save/quit/reload and confirm that
lesson mode still activates. The native full tutorial starts its task sequence
again each session; this change does not add exact task checkpoint persistence.
Check that an ordinary save still launches without full tutorial mode. Test a
second local player and both other dimensions for misplaced hints. Remote-client
lesson synchronization and actual in-game progression remain unverified.
These are adapted native lessons, not a claim of exact TU46 popup/script parity.
