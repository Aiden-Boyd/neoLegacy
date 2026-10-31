fn press(game: &mut crate::game::Game<'_>, right: bool) -> ::core::ops::ControlFlow<()> {
    if crate::game::lce_glue::active(game) && crate::game::lce_glue::press(game, right) {
        game.inv_clicks.skip_release();
        return ::core::ops::ControlFlow::Break(());
    }
    ::core::ops::ControlFlow::Continue(())
}
