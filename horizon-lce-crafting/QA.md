# Validation

Automated checks completed are recorded in `VALIDATION.md`.

Still requires a real Horizon instance with the player's prepared Minecraft assets:

- Launch the native build on macOS, Windows and Linux; open inventory and a crafting table.
- Check low resolution, GUI scale changes, resize, item tooltips and all seven categories.
- Craft planks, sticks, a pickaxe and bread; test recipes with overlapping ingredient tags and container remainders.
- Craft once with several ingredient sets already in the grid, then Craft All.
- Test full/nearly full inventory, a held cursor stack, split ingredient stacks and missing ingredients.
- Verify normal click, shift-click, dragging, hotbar keys, armour and offhand handling.
- Switch to the normal menu, close, and reopen; verify the LCE screen returns.
- Close or switch menus during a pending craft; disconnect/rejoin and change dimensions.
- Test normal server-thread single player, LAN and an unmodified Java server with latency.
- Check compatibility with the official Recipes native mod and other screen mods; the custom drawing path intentionally replaces the inventory background.

Do not label the alpha playtested until these have been observed in the game.
