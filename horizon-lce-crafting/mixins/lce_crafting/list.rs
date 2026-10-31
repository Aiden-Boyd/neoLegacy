fn list(mods: &mut Vec<crate::game::native_hooks::NativeMod>) {
    crate::game::lce_glue::register(mods);
}
