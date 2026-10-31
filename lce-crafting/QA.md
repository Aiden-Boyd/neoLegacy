# In-game acceptance checks (not yet executed)

- Launch 26.3 with only Fabric Loader and this mod; verify both mixins apply.
- Open survival inventory and crafting table; resize and change GUI scale; verify all slots and tabs remain usable.
- Craft planks, sticks, a pickaxe, bread and a recipe with bucket/bottle remainders.
- Test Craft with several sets already in the grid: only one recipe operation should be taken.
- Test Craft all with ingredients split across inventory slots, nearly full inventory, full inventory and remainder items.
- Verify missing ingredients cannot produce output; 3×3-only recipes do not appear in inventory crafting.
- Test dragging, shift-click, number-key swaps, armour, offhand and returning to Java menu.
- Test on an unmodified server with latency, server recipe restrictions and a datapack reload.
- Close the screen, change menus or disconnect while a craft request is pending; confirm no delayed action targets the next container.
- Test a standard modded shaped and shapeless recipe, different ingredient alternatives and recipe unlocks.
- Verify death, respawn, creative/survival transitions and reopening inventory preserve slot positions.

Known alpha limitations: server-unlocked recipes only; heuristic families/categories; no controller integration; artwork is an original simple layout, not an exact texture recreation; no per-ingredient missing-count labels yet.
