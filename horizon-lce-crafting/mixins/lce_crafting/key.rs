fn lce_key(
    game: &mut crate::game::Game<'_>,
    key: winit::keyboard::KeyCode,
) -> ::core::ops::ControlFlow<Result<()>> {
    if crate::game::lce_glue::active(game) && crate::game::lce_glue::key(game, key) {
        return ::core::ops::ControlFlow::Break(Ok(()));
    }
    ::core::ops::ControlFlow::Continue(())
}
