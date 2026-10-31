# LCE Crafting for Horizon — 26.3 alpha

A native Rust mod for **Horizon**, the Rust Minecraft client. It opens a Legacy Console inspired category/variant crafting interface in the survival inventory and crafting table. This is a `.hnmod` source bundle, not a Fabric JAR.

## Install

1. Download `dist/lce-crafting-0.1.0-horizon-26.3.hnmod`.
2. Put it in `horizon-native-mods/` in your Horizon instance's game folder.
3. In the Horizon launcher, open **Settings → Game → Native mods (experimental) → Build**. Horizon compiles a game with this mod installed. Then launch that instance.

The bundle pins the exact Horizon and horizon-mixins commits in `compatibility.json`. It uses game screen hooks in addition to native hook points, so it does **not** claim compatibility with every Horizon nightly. Other native mods that pin a different game revision may conflict. The launcher performs the native build; this file cannot be loaded as a content `.hmod` or Java `.jar`.

## What it does

- Structures, Tools, Food, Armour, Mechanisms, Transport and Decoration categories.
- Horizontal item families with vertical material/colour variants; ten families per page.
- 2×2 inventory and 3×3 crafting-table recipe filtering.
- Ingredient previews, normal inventory/armour/offhand slots, Craft and Craft All.
- A **Java menu** button restores Horizon's normal screen until the container closes.
- Creative inventory and other container screens retain their existing UI.

Click tabs/families/variants to navigate, or use **Tab**, **left/right** and **up/down**. **Enter** crafts once; **Shift+Enter** crafts all. The `<`/`>` buttons move between pages.

The mod uses Horizon's normal recipe-placement and menu-click paths. It waits for an authoritative update to the current container and checks the selected recipe's output before collecting it. It cancels on screen closure, menu changes, a nonempty cursor or a five-second timeout. A single craft takes one result; if your inventory fills, the result can stay safely on the cursor. No stack is created by the mod, and no server installation is required.

## Alpha limits

This recreates the workflow with an original simple UI; it is not yet pixel-identical LCE artwork. Grouping is heuristic. Missing ingredient counts, controller navigation, translated mod labels and recipe search are not implemented. Crafting requires the player's unlocked recipe book. Recipe data is limited to what Horizon exposes: a vanilla server does not provide arbitrary datapack recipes through this API. The deprecated in-frame single-player path is not supported; normal server-thread single player and multiplayer use the server acknowledgement path.

No in-game visual or multiplayer playtest has been performed. Use the acceptance list in `QA.md` before treating this as a release.

## Develop

```sh
cargo test
cargo clippy --all-targets -- -D warnings
python3 tools/package.py
```

With the exact pinned Horizon source and horizon-mixins CLI:

```sh
horizon-mixins apply --workspace /path/to/horizon --mods ./mixins --out /tmp/lce-horizon
cargo check --locked --manifest-path /tmp/lce-horizon/Cargo.toml -p horizon-client --bin horizon-client --no-default-features
cargo horizon-mod check dist/lce-crafting-0.1.0-horizon-26.3.hnmod
```

An exported source archive needs its `.horizon-revision` file containing the exact commit, as the launcher writes; a Git checkout uses HEAD. The mixin tool requires its cached dependencies (`cargo fetch --locked`) and does not accept symlinks in staged input. On an exported archive, materialise `CLAUDE.md` from its target without changing its text before applying.

`runtime.rs` contains the standalone model and tests. `glue.rs` contains Horizon integration. The remaining Rust files are typed source-hook adapters. `tools/package.py` produces a deterministic source-only bundle and checks manifest identity and referenced files.

## References and licence

Original implementation, MIT. No Minecraft assets, game binaries, Neo Legacy C++ or console textures are bundled. Category order and behaviour follow the LCE interface requested for this project. Horizon APIs and the native bundle format were checked against the pinned MIT-licensed Horizon and horizon-mixins source; no changes are made to the upstream repositories.
