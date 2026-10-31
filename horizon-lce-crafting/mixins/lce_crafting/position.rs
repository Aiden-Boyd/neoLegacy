fn position(game: &crate::game::Game<'_>, i: usize) -> ::core::ops::ControlFlow<(f32, f32)> {
    if crate::game::lce_glue::active(game) {
        return ::core::ops::ControlFlow::Break(
            crate::game::lce_runtime::slot_position(i, game.craft_size)
                .unwrap_or((-1000.0, -1000.0)),
        );
    }
    ::core::ops::ControlFlow::Continue(())
}
