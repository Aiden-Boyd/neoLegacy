fn bookclick(game: &mut crate::game::Game<'_>) -> ::core::ops::ControlFlow<bool> {
    if crate::game::lce_glue::active(game) {
        return ::core::ops::ControlFlow::Break(false);
    }
    ::core::ops::ControlFlow::Continue(())
}
