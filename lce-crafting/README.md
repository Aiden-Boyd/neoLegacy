# LCE Crafting — Fabric 26.3 alpha

A client-only recreation of Legacy Console Edition's category-and-variant crafting menu. This is an independent Java implementation, with the behaviour and category order referenced from neoLegacy's `IUIScene_CraftingMenu`. No original C++ code or console textures are bundled.

## Install

Use Minecraft **26.3**, Fabric Loader **0.20.1 or newer**, and Java **25**. Put `dist/lce-crafting-0.1.0+26.3-alpha.jar` in your instance's `mods` directory. Fabric API is not required. The server does not need this mod.

Opening survival inventory or a crafting table opens the menu. Creative inventory remains vanilla. The **Java menu** button restores the original screen without closing the active crafting container.

## Controls

- Tabs select Structures, Tools, Food, Armour, Mechanisms, Transport, or Decoration.
- Click a recipe family; use up/down or the mouse wheel to select its variants.
- Left/right arrows move between families; the `<` and `>` buttons change pages.
- **Craft** or Enter crafts one recipe operation. **Craft all** or Shift+Enter requests maximum recipe placement and shift-transfers the result.
- Inventory slots retain vanilla mouse, drag and hotbar controls. Automatic crafting briefly locks slot edits while awaiting a server update.

The menu uses recipes unlocked and sent by the server. Standard shaped/shapeless mod recipes can appear; special/dynamic recipes may require the Java menu. It respects the inventory's 2×2 grid and a table's 3×3 grid. Material/colour grouping and mod category placement are heuristic in this alpha. Controller support and exact LCE artwork are not implemented yet.

## Item handling

Only vanilla recipe-placement and container-input requests are used. The server validates ingredients and creates results. The mod waits for a changed container state and matching output before collecting it, and cancels on timeout, cursor changes, screen closure or menu changes. Single crafting picks up once and deposits through normal inventory clicks. If inventory space runs out the result may remain on the cursor, as in vanilla. Closing and switching screens restore the original slot coordinates.

## Build

```sh
cd lce-crafting
./gradlew build
```

Windows: `gradlew.bat build`. The Gradle project uses Loom 1.17.21, Gradle 9.6.0 and Java 25. Output is in `build/libs/`.

## Verification status

The alpha was compiled with Java 25 against the official unobfuscated 26.3 client and its declared libraries; metadata and packaged mixin class checks passed. The normal Loom build was blocked by the execution environment denying its Unix-domain socket capability check. The bundled alpha was packaged directly from the compiled classes. **No in-game or multiplayer playtest has been completed.** Treat this as a prototype, not a finished release. See `QA.md` for the remaining checks.

## Reference

- `Minecraft.Client/Common/UI/IUIScene_CraftingMenu.h`
- `Minecraft.Client/Common/UI/IUIScene_CraftingMenu.cpp`

The native neoLegacy project is separate from this standalone Gradle module.
