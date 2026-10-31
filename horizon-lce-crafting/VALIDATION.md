# Validation record

- Horizon: `3faaef6693ea33383fbac44d0407a7bcf817d489` (Minecraft 26.3).
- horizon-mixins: `4342a0bfbe71d4d14185528d64e496d6022f16a1`.
- Toolchain: Rust/Cargo 1.99.1, edition 2024.
- `cargo test`: **9 passed**, covering ingredient alternatives, no ingredient double-counting, grid constraints, variant selection, menu/window/cursor/timeout cancellation, acknowledgement gating, slot bounds and toolbar hit bounds.
- Standalone `cargo clippy --all-targets -- -D warnings`: **passed**.
- `rustfmt --edition 2024` applied to every bundle Rust file.
- Native source transformation: **passed**, 13 HEAD injections and 2 support modules on the exact pinned source; originals retained separately.
- Full staged game `cargo check --locked --offline -p horizon-client --bin horizon-client --no-default-features`: **passed**, no warnings on the final sources. Audio/controller feature integrations were not compiled in this check.
- Horizon's own `cargo-horizon-mod check`: **passed**, correct native manifest, source bundle and pinned revisions.
- Deterministic packaging and ZIP integrity: **passed**. Bundle SHA-256: `df65d7e7b5e2840b2d0fd545bc6e4912f2c3f25f0b12ecc1acfc17f4342b04ae`.

Strict full-game Clippy with Rust 1.99 initially reports existing upstream `collapsible_if` / `if_same_then_else` findings. The upstream game source was not changed to fix those unrelated lints. The standalone mod model passes strict Clippy without allowances.

**Not completed:** launch/playtesting, visual review, latency/full-inventory/remainder tests on a real server, and macOS/Windows native builds. No prepared Minecraft resources or game assets are included. See `QA.md`.
