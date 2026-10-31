fn draw(
    game: &mut crate::game::Game<'_>,
    batch: &mut horizon_client::hud::HudBatch,
) -> ::core::ops::ControlFlow<()> {
    if crate::game::lce_glue::active(game) {
        crate::game::lce_glue::draw(game, batch);
        return ::core::ops::ControlFlow::Break(());
    }
    ::core::ops::ControlFlow::Continue(())
}
