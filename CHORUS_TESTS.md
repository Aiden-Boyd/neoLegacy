# Chorus and end rods: Steam/Proton candidate

Branch: `feature/chorus-end-rods`. Based on the portal projection candidate; the accepted `steam-proton-base` remains available.

Implemented: connected chorus plant geometry and collision, flowers with age/growth/branching/death and support checks, fruit drops, edible chorus fruit with safe host-side teleport and a 20-tick cooldown, smelting to popped fruit, four popped fruit into four purpur blocks, and one blaze rod above one popped fruit into four end rods. End rods have six orientations, reversed paired placement, thin collision, light level 14 and white fading particles. New names, hints, Creative entries, vanilla textures and terrain mipmaps are included.

Natural outer End islands and chorus generation are not included in this milestone. Start a farm using a Chorus Flower from Creative on End Stone. This is a CU59-targeted implementation; compiling it does not establish exact CU59 parity. Teleport sound currently reuses the engine's Enderman portal sound.

## Automated validation

`python tests/chorus_regression.py` compiles the actual new tile/item implementations against deterministic engine fixtures. It checks six rod orientations and paired reversal, upward growth, branching, dead flowers, support loss, flower harvest versus indirect breaking, live/dead textures, connections, food use/cooldown/Creative counts, and teleport rejection for unloaded chunks, collisions and liquids. These fixtures do not exercise the renderer, real network transport or saves. CI also runs all earlier regressions and compiles the full Windows64 client.

## In-game acceptance (not yet verified)

Use a copied test world and the default texture pack. Run the executable through the existing Steam/Proton setup.

1. Open every Creative tab with controller and mouse. Confirm Chorus Plant, Chorus Flower and End Rod under Decorations, Chorus Fruit under Food, and Popped Chorus Fruit under Materials; verify names, icons, held models and pagination.
2. Plant several flowers on End Stone in a well-lit open area. Wait for growth: stems should connect, flowers should rise and branch, and mature flowers should appear darker. Flowers must not grow on ordinary stone or accept unsupported placement. Grow beside walls and near the world ceiling.
3. Break the bottom of a grown plant: unsupported sections should collapse. Stems may drop fruit; directly harvested flowers should drop one flower regardless of age; indirect collapse should not drop flowers. Test save/reload during growth and after maturity.
4. Eat fruit in Survival at full and partial hunger; check the normal eating animation, four hunger points and nearby teleport. Hold Use continuously to test the cooldown. Test Creative counts, water, cramped rooms, cliffs, unloaded boundaries, riding and a second player. Failed searches must leave the player in place. Successful teleports must not land inside a block or liquid or cause accumulated fall damage. Check the destination seen by a remote client.
5. Shift-click fruit into a furnace and smelt it. Confirm one popped fruit per input. Craft a 2x2 square of popped fruit into four purpur blocks; craft a blaze rod above popped fruit into four rods. Test both Classic and console crafting menus, including recipe discovery and required ingredients.
6. Place rods on all six faces, then extend each with a second rod: paired bases should face away from each other. Check lighting at night, white particles, selection/collision, inventory and held-item appearance, support removal, breaking and save/reload. Check thin geometry from above/below and all sides; no invisible or inside-out faces.
7. Recheck the portal from different viewing angles, Elytra/head rendering, sponge inventory and wet-sponge furnace transfer. Record a Proton log for any crash.

Keep the candidate separate until these checks pass. Exact visual matching, controller use, multiplayer behavior, alternate texture packs and piston interactions need runtime acceptance.
