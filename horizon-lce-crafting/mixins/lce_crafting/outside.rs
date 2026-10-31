fn outside(
    game: &crate::game::Game<'_>,
    batch: &horizon_client::hud::HudBatch,
) -> ::core::ops::ControlFlow<bool> {
    if crate::game::lce_glue::active(game) {
        return ::core::ops::ControlFlow::Break({
            let (x, y) = crate::game::lce_glue::origin(batch);
            game.cursor.0 < x
                || game.cursor.1 < y
                || game.cursor.0 >= x + 300.0
                || game.cursor.1 >= y + 232.0
        });
    }
    ::core::ops::ControlFlow::Continue(())
}
