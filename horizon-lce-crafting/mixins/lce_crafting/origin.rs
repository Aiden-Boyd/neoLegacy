fn origin(
    game: &crate::game::Game<'_>,
    batch: &horizon_client::hud::HudBatch,
) -> ::core::ops::ControlFlow<(f32, f32)> {
    if crate::game::lce_glue::active(game) {
        return ::core::ops::ControlFlow::Break(crate::game::lce_glue::origin(batch));
    }
    ::core::ops::ControlFlow::Continue(())
}
