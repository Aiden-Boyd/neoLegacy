# End cities, ships and shulkers: Steam/Proton candidate

Branch: `feature/end-cities-shulkers`. Includes the previous gateway, outer-island, chorus, palette and stability changes. The accepted Steam/controller baseline and gateway build remain on their separate branches.

## Implemented

New outer-End population can place seeded cities built from all 21 original reference room/bridge/tower/ship templates. Four terrain samples reject low/void sites. Rotations transform both coordinates and directional block metadata. Layout and treasure random streams are local; population order does not consume the terrain generator's random stream. Each placement is clipped to its population chunk. The End-level metadata wrapper delegates a saved per-chunk population journal to the root world data, preventing repeat population from refilling loot or duplicating sentries and frames.

Ships have an Elytra frame, treasure chests, two strong-healing potions, shulker sentries and a collectible dragon head. Natural city banners preserve the reference green/black design as a dedicated collectible wall-banner block. They can be harvested and placed again; this is not a general banner-pattern crafting system.

Shulkers have 30 health, armor while closed, closed-shell arrow immunity, six attachment directions, opening shells, teleportation, ranged guided bullets, levitation and shell drops. Registry/save data, mob/object spawn packets, tracking, textures, original sound variants and a Creative spawn egg are included. Bullets can be destroyed and save their owner/target UUIDs, direction and steering state. Levitation uses effect ID 25 and integrates into living-entity gravity. Creative also exposes shells, city banners and dragon heads.

## Scope and parity

This is a playable feature candidate, not certification of complete Xbox One CU59 parity. Room geometry uses the Java 1.12.2 reference assets; the bounded layout planner and site selection are this fork's implementation, so city layouts/locations are not guaranteed to match an original Xbox or Java seed. Cities only appear through new population; existing populated terrain is preserved.

Loot uses the reference weights and count/level ranges for supported items. Beetroot seeds are omitted because agriculture is not implemented in this base. Enchantments use the engine's existing helper; missing treasure enchantments such as Mending are not implemented here. Shells are present, but shulker boxes are a separate feature. Levitation displays its name/duration in the existing effects UI; its dedicated Iggy effect icon remains absent. Natural banners have the fixed city pattern. Dragon-head redstone jaw animation is not included.

## Automated checks

Run `python tests/end_city_templates_regression.py`, `python tools/generate_end_city_header.py --check`, and `python tests/end_city_regression.py`. They check all imported cells/markers, metadata rotation cycles, deterministic bounded layouts, opposite chunk-generation orders, loot/ship contents, population journals and metadata delegation, six shell directions, armor/arrow behavior, ranged attacks, projectile damage/levitation, client/peaceful guards and invalid saved steering. Full Windows compilation checks integration with the actual engine APIs. Fixtures do not replace acceptance in the game.

## In-game acceptance still required

Use a copied world or a new disposable world and the default texture pack. Both players must run the same candidate for multiplayer.

1. Follow a gateway into newly generated outer terrain. Find cities and ships. Approach from different directions and cross chunk boundaries; look for complete roofs, stairs, bridges, rods, glass and banner faces. Check that the main island and older populated areas remain intact.
2. In Survival, fight shulkers with melee and arrows while their shells are closed/open. Verify bullet steering, bullet destruction, levitation, fall behavior after expiration, shell drops, opening/closing/shooting/hurt/death sounds, support changes and teleportation. In Peaceful, hostile mobs/projectiles must not remain active. Check Creative spawn eggs on floors, ceilings and walls.
3. Collect the Elytra, dragon head, banners, potions and chest loot. Equip/fly with the head and Elytra. Check that the brewing stand contains Healing II, that the frame drops its Elytra, and that controller inventory/quick-move behavior remains usable.
4. Save and quit after looting part of a city. Reload, leave/return through a gateway, and approach from another direction. Sentries should retain their saved state, emptied chests should stay empty, and collected frames/heads/banners must not respawn. Save/reload with active bullets and levitation as well.
5. Have a second player join after the city already exists. Verify visible shulkers, attachment/shell updates, bullets, dragon heads, banners, combat and save/reload on both clients.
6. Recheck prior portal projection, sponge/furnace, chorus, rod, gateway and Steam/controller behavior. Record the seed, coordinates, BUILD_INFO.txt and Proton log for failures.
