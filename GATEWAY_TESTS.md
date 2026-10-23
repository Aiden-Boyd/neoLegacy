# End gateways: Steam/Proton candidate

Branch: `feature/end-gateways`, based on the outer-islands candidate. Cities, ships and shulkers are separate work; this package is the gateway checkpoint.

## Behavior

Defeating the dragon creates one gateway in a seeded sequence of up to twenty positions around the central island. A saved per-world counter tracks the sequence, including through the derived End-level metadata wrapper. For a previously defeated world, an already-loaded active central exit portal triggers creation of its first gateway if its counter is still zero.

Gateways have bedrock frames, a six-sided animated star surface, a spawn beam and periodic cooldown beams. The surface uses explicit face UVs rather than dividing by a camera-plane distance. Initial activation takes 200 ticks; transport starts a 40-tick cooldown. Gateway state is saved, cloned and sent through a dedicated tile-entity update packet.

First outbound transport creates a linked return gateway in the outer End. This finite-world candidate targets approximately 1280 blocks from the center, finds nearby End Stone or creates a small fallback island, and records the bidirectional link. Exits prefer clear space above nearby solid terrain; the bedrock cap is a fallback. Collision and liquid checks run before moving the traveler. Ender pearls transport their living owner through the player's networked teleport path; gateway transport does not apply ordinary pearl fall damage. Mounted entities and dragons cannot enter. Natural one-way return gateways may also appear during new outer-chunk population.

This is a CU59-targeted implementation, not a claim of exact original gateway placement, exit-search or visual parity. Existing populated chunks are not repopulated with natural return gateways.

## Automated validation

`python tests/gateway_regression.py` compiles the production gateway implementation and renderer against fixtures. It checks frames, activation, cooldown, outbound/return links, save/load and cloning, update packets, invalid saved destinations, blocked/liquid exits, pearl ownership, rider/client rejection, twenty unique central positions, legacy-world migration, metadata delegation and renderer face winding/finite coordinates at different portal-plane heights. The full client build checks integration with the engine. Real storage, controller play, rendering and multiplayer require acceptance below.

## In-game acceptance (not yet verified)

Use a copied test world, the default texture pack and the existing Steam/Proton setup. Both players must use this candidate for a multiplayer test.

1. Defeat the dragon in a fresh End. Verify the central exit portal, XP and egg still appear, plus one bedrock gateway around radius 96 at Y=75. Observe the magenta spawn beam, wait ten seconds, and inspect all sides of the surface from above, below and at the same height.
2. Throw a pearl through the gateway with the controller. Confirm that the player arrives safely on an outer island or the fallback cap, that a return gateway exists, and that transport does not inflict ordinary pearl damage. Wait for activation/cooldown before returning.
3. Repeat with Creative flight, normal walking/crawling access, items and mobs. Mounted entities and dragons should stay in place. Throw a pearl during activation/cooldown: it must not teleport the player into the bedrock frame. Repeat rapid entry attempts to check for loops.
4. Save and quit after both directions have been used. Reload and repeat the round trip. Verify the links, frame, nearby terrain and player inventory persist. Test the second player entering independently, then after joining a world with an existing linked gateway.
5. On a copy of an older defeated world, visit the active central exit portal. Confirm exactly one initial gateway appears, then save/reload and verify it is not duplicated. A world with an undefeated dragon and inactive podium must not receive this migration gateway.
6. Obstruct the terrain exits, add liquids, and check that the destination cap remains safe. Completely obstruct headroom above the destination and verify transport fails without moving the player. Reopen headroom and retry after cooldown.
7. Explore new outer chunks and look for natural return gateways. Check their center-island return. Recheck earlier terrain, chorus, end rods, portal projection, Elytra/heads and sponge behavior.

Dragon resummoning and an exact CU59 dragon-fight audit are not part of this gateway checkpoint. Repeat-dragon sequencing can only be accepted once the existing project's respawn path is verified. Record seed, coordinates, BUILD_INFO.txt and a Proton log for crashes or transport failures.
