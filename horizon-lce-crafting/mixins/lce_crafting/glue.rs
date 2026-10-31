// Horizon-specific UI edges; all crafting goes through the game's existing menu path.
use super::lce_runtime as m;
use super::native_hooks::{NativeMod, NativeRecipes};
use super::{Game, Screen};
use horizon_client::{hud::HudBatch, menu::Input};
use std::{cell::RefCell, collections::BTreeMap, sync::Arc, time::Instant};
use winit::keyboard::KeyCode;
const ID: &str = "lce_crafting";
struct State {
    menu: m::Menu,
    tables: Option<Arc<NativeRecipes>>,
    recipes: Vec<m::Recipe>,
    pending: Option<m::Pending>,
    layout_key: Option<(usize, usize)>,
    ack: u64,
    vanilla: bool,
    start: Instant,
    status: String,
}
impl Default for State {
    fn default() -> Self {
        Self {
            menu: m::Menu::default(),
            tables: None,
            recipes: vec![],
            pending: None,
            layout_key: None,
            ack: 0,
            vanilla: false,
            start: Instant::now(),
            status: "Choose a recipe".into(),
        }
    }
}
thread_local! { static STATE: RefCell<State> = RefCell::new(State::default()); }
pub(super) fn active(game: &Game<'_>) -> bool {
    game.screen == Screen::Inventory
        && game.player.is_some()
        && game.container.is_none()
        && game.station.is_none()
        && !game.mount_active()
        && [2, 3].contains(&game.craft_size)
        && game.native_on(ID)
        && STATE.with(|s| !s.borrow().vanilla)
}
pub(super) fn register(mods: &mut Vec<NativeMod>) {
    mods.push(NativeMod {
        id: ID.into(),
        title: "LCE Crafting 0.1.0".into(),
        description: "Legacy Console category and variant crafting".into(),
        keys: vec![],
    });
}
pub(super) fn reset() {
    STATE.with(|s| *s.borrow_mut() = State::default());
}
pub(super) fn ack(game: &Game<'_>, window: i32) {
    if game.remote_menu_id() == window {
        STATE.with(|s| {
            let mut s = s.borrow_mut();
            s.ack = s.ack.wrapping_add(1);
        });
    }
}
pub(super) fn origin(batch: &HudBatch) -> (f32, f32) {
    batch.centered_origin(m::WIDTH, m::HEIGHT)
}
fn stock(game: &Game<'_>) -> BTreeMap<String, u32> {
    let mut out = BTreeMap::new();
    for stack in game
        .inventory
        .iter()
        .chain(game.craft_grid.iter())
        .flatten()
    {
        *out.entry(stack.item.to_string()).or_default() += stack.count;
    }
    out
}
fn ensure(game: &mut Game<'_>) {
    let Some(tables) = game.native_recipes() else {
        return;
    };
    STATE.with(|s| {
        let mut s = s.borrow_mut();
        if !s.tables.as_ref().is_some_and(|t| Arc::ptr_eq(t, &tables)) {
            s.recipes = tables
                .recipes
                .iter()
                .map(|r| {
                    let out = r
                        .results
                        .first()
                        .and_then(|s| tables.items.get(s.item as usize));
                    m::Recipe {
                        id: r.id.clone(),
                        book: r.book_id,
                        shaped: matches!(
                            r.kind,
                            super::native_hooks::NativeRecipeKind::CraftingShaped
                        ),
                        width: r.width as usize,
                        height: r.height as usize,
                        category: r.category.clone(),
                        group: r.group.clone(),
                        output: out.map_or_else(String::new, |i| i.id.clone()),
                        name: out.map_or_else(String::new, |i| i.name.clone()),
                        count: r.results.first().map_or(0, |s| s.count),
                        ingredients: r
                            .slots
                            .iter()
                            .map(|slot| {
                                slot.iter()
                                    .filter_map(|s| {
                                        tables.items.get(s.item as usize).map(|i| i.id.clone())
                                    })
                                    .collect()
                            })
                            .collect(),
                        food: out
                            .and_then(|i| game.items().get(&i.id))
                            .is_some_and(|i| i.food.is_some()),
                    }
                })
                .collect();
            // The native tables also contain cooking/station recipes; they must never enter a crafting menu.
            for (i, r) in tables.recipes.iter().enumerate() {
                if !matches!(
                    r.kind,
                    super::native_hooks::NativeRecipeKind::CraftingShaped
                        | super::native_hooks::NativeRecipeKind::CraftingShapeless
                ) {
                    s.recipes[i].book = -1;
                }
            }
            s.tables = Some(tables);
            s.layout_key = None;
        }
        let key = (game.craft_size, s.menu.tab);
        if s.layout_key != Some(key) {
            let State { menu, recipes, .. } = &mut *s;
            menu.rebuild(recipes, game.craft_size);
            s.layout_key = Some(key);
        }
    });
}
pub(super) fn request(game: &mut Game<'_>, all: bool) {
    ensure(game);
    let known = game.native_known_recipes();
    let available = stock(game);
    let request = STATE.with(|state| {
        let mut s = state.borrow_mut();
        let index = s.menu.selected()?;
        let r = &s.recipes[index];
        if s.pending.is_some() || game.carried.is_some() {
            return None;
        }
        if !known.contains(&r.book) {
            s.status = "Recipe is not unlocked yet".into();
            return None;
        }
        if !m::can_craft(r, &available) {
            s.status = "Missing ingredients".into();
            return None;
        }
        let book = r.book;
        s.pending = Some(m::Pending {
            window: game.remote_menu_id(),
            recipe: index,
            all,
            ack: s.ack,
            started_ms: s.start.elapsed().as_millis() as u64,
        });
        s.status = "Waiting for server...".into();
        Some(book)
    });
    if let Some(book) = request
        && let Err(e) = game.native_place_recipe(book, all)
    {
        STATE.with(|s| {
            let mut s = s.borrow_mut();
            s.pending = None;
            s.status = e;
        });
    }
}
pub(super) fn frame(game: &mut Game<'_>) {
    if game.screen != Screen::Inventory {
        reset();
        return;
    }
    if !active(game) {
        STATE.with(|s| s.borrow_mut().pending = None);
        return;
    }
    let take = STATE.with(|s| {
        let mut s = s.borrow_mut();
        let p = s.pending.as_ref()?;
        let r = &s.recipes[p.recipe];
        let ready = game.craft_recipe().is_some_and(|(id, result)| {
            id == r.id && result.item.as_ref() == r.output && result.count == r.count
        });
        // The obsolete in-frame path has no network acknowledgement. Only server-thread clients craft automatically.
        match p.poll(
            game.remote_menu_id(),
            s.start.elapsed().as_millis() as u64,
            s.ack,
            ready,
            game.carried.is_none(),
        ) {
            m::Decision::Wait => None,
            m::Decision::Cancel => {
                s.pending = None;
                s.status = "Craft cancelled or timed out".into();
                None
            }
            m::Decision::Take => {
                let all = p.all;
                s.pending = None;
                Some(all)
            }
        }
    });
    if let Some(all) = take {
        game.menu_click(
            Some(200),
            0,
            if all { Input::QuickMove } else { Input::Pickup },
        );
        if !all {
            for i in 0..36 {
                let Some(carried) = game.carried.as_ref() else {
                    break;
                };
                let fits = game.inventory[i].as_ref().is_none_or(|stack| {
                    stack.item == carried.item
                        && stack.damage == carried.damage
                        && stack.extra == carried.extra
                        && stack.count < game.items().get(&stack.item).map_or(64, |i| i.max_stack)
                });
                if fits {
                    game.menu_click(Some(i), 0, Input::Pickup);
                }
            }
        }
        STATE.with(|s| {
            s.borrow_mut().status = if game.carried.is_some() {
                "Result is on the cursor"
            } else {
                "Craft requested"
            }
            .into()
        });
    }
}
fn button(
    game: &Game<'_>,
    batch: &mut HudBatch,
    x: f32,
    y: f32,
    w: f32,
    label: &str,
    selected: bool,
) {
    game.native_fill(
        batch,
        [x, y, w, 18.0],
        if selected { 0xFFEFEFEF } else { 0xFF929292 },
    );
    game.native_text(batch, label, [x + 3.0, y + 5.0], 0x303030, false);
}
pub(super) fn draw(game: &mut Game<'_>, batch: &mut HudBatch) {
    ensure(game);
    let (ox, oy) = origin(batch);
    let known = game.native_known_recipes();
    let available = stock(game);
    game.native_fill(batch, [0.0, 0.0, batch.width, batch.height], 0xA0101010);
    game.native_fill(batch, [ox, oy, m::WIDTH, m::HEIGHT], 0xFFBEBEBE);
    STATE.with(|shared| {
        let state = shared.borrow();
        let (menu, recipes, pending, status) = (
            &state.menu,
            &state.recipes,
            state.pending.is_some(),
            &state.status,
        );
        game.native_text(
            batch,
            m::TABS[menu.tab],
            [ox + 8.0, oy + 5.0],
            0x303030,
            false,
        );
        game.native_text(batch, status, [ox + 78.0, oy + 5.0], 0x303030, false);
        for (i, name) in [
            "Build", "Tools", "Food", "Armour", "Mech.", "Travel", "Decor",
        ]
        .iter()
        .enumerate()
        {
            button(
                game,
                batch,
                ox + 8.0 + i as f32 * 41.0,
                oy + 17.0,
                40.0,
                name,
                menu.tab == i,
            );
        }
        let page = menu.family / 10;
        for (i, group) in menu.groups.iter().enumerate().skip(page * 10).take(10) {
            let x = ox + 8.0 + (i % 10) as f32 * 25.0;
            game.native_fill(
                batch,
                [x, oy + 40.0, 23.0, 22.0],
                if i == menu.family {
                    0xFFFFFFFF
                } else {
                    0xFF999999
                },
            );
            game.native_item(batch, &recipes[group[0]].output, [x + 3.0, oy + 43.0], 16.0);
        }
        button(game, batch, ox + 261.0, oy + 40.0, 14.0, "<", false);
        button(game, batch, ox + 277.0, oy + 40.0, 14.0, ">", false);
        for slot in game.slots() {
            if let Some((x, y)) = m::slot_position(slot, game.craft_size) {
                game.native_fill(batch, [ox + x - 1.0, oy + y - 1.0, 18.0, 18.0], 0xFF777777);
            }
        }
        if let Some(index) = menu.selected() {
            let r = &recipes[index];
            let mut title = r.name.clone();
            while game.native_text_width(&title) > 210.0 {
                title.pop();
            }
            game.native_text(batch, &title, [ox + 70.0, oy + 65.0], 0x303030, false);
            let group = &menu.groups[menu.family];
            for i in -1isize..=1 {
                let v = (menu.variant as isize + i).rem_euclid(group.len() as isize) as usize;
                let y = oy + 96.0 + i as f32 * 20.0;
                game.native_fill(
                    batch,
                    [ox + 19.0, y - 1.0, 18.0, 18.0],
                    if i == 0 { 0xFFFFFFFF } else { 0xFF999999 },
                );
                game.native_item(batch, &recipes[group[v]].output, [ox + 20.0, y], 16.0);
            }
            let rw = if r.shaped { r.width } else { game.craft_size };
            for (i, options) in r.ingredients.iter().enumerate() {
                let gi = i / rw * game.craft_size + i % rw;
                if game.craft_grid[gi].is_none()
                    && let Some(item) = options.first()
                {
                    let (x, y) = m::slot_position(100 + gi, game.craft_size).unwrap();
                    game.native_item(batch, item, [ox + x, oy + y], 16.0);
                    game.native_fill(batch, [ox + x, oy + y, 16.0, 16.0], 0x66888888);
                }
            }
            let can = known.contains(&r.book)
                && m::can_craft(r, &available)
                && !pending
                && game.carried.is_none();
            if game.craft_result().is_none() {
                game.native_item(batch, &r.output, [ox + 194.0, oy + 95.0], 16.0);
            }
            button(game, batch, ox + 218.0, oy + 110.0, 72.0, "Craft", can);
            button(game, batch, ox + 218.0, oy + 134.0, 72.0, "Craft all", can);
        }
        button(
            game,
            batch,
            ox + 218.0,
            oy + 185.0,
            72.0,
            "Java menu",
            false,
        );
        game.native_text(batch, "Inventory", [ox + 8.0, oy + 138.0], 0x303030, false);
        game.draw_hovered_slot_highlight(batch, false, ox, oy);
        game.draw_inventory_slots(batch, ox, oy);
        game.draw_hovered_slot_highlight(batch, true, ox, oy);
        game.draw_carried(batch);
        game.draw_slot_tooltip(batch);
    });
}
pub(super) fn press(game: &mut Game<'_>, right: bool) -> bool {
    let Some(renderer) = game.renderer.as_ref() else {
        return false;
    };
    let batch = HudBatch::new([renderer.config.width, renderer.config.height]);
    let (ox, oy) = origin(&batch);
    let (x, y) = (game.cursor.0 - ox, game.cursor.1 - oy);
    if STATE.with(|s| s.borrow().pending.is_some()) {
        return true;
    }
    let toolbar = m::toolbar_hit(x, y);
    if !toolbar {
        return false;
    }
    game.inv_clicks.skip_release();
    if right {
        return true;
    }
    if (17.0..35.0).contains(&y) && (8.0..295.0).contains(&x) {
        STATE.with(|s| {
            let mut s = s.borrow_mut();
            s.menu.tab = ((x - 8.0) / 41.0) as usize;
            s.menu.family = 0;
            s.menu.variant = 0;
        });
        ensure(game);
    } else if (40.0..62.0).contains(&y) {
        STATE.with(|s| {
            let mut s = s.borrow_mut();
            if x >= 261.0 {
                let dir = if x < 276.0 { -10 } else { 10 };
                s.menu.move_family(dir);
            } else if x >= 8.0 {
                let i = s.menu.family / 10 * 10 + ((x - 8.0) / 25.0) as usize;
                if i < s.menu.groups.len() {
                    s.menu.family = i;
                    s.menu.variant = 0;
                }
            }
        });
    } else if x < 42.0 {
        STATE.with(|s| {
            s.borrow_mut()
                .menu
                .move_variant(if y < 95.0 { -1 } else { 1 })
        });
    } else if (110.0..130.0).contains(&y) {
        request(game, false);
    } else if (134.0..154.0).contains(&y) {
        request(game, true);
    } else if (185.0..205.0).contains(&y) {
        STATE.with(|s| s.borrow_mut().vanilla = true);
    }
    true
}
pub(super) fn key(game: &mut Game<'_>, key: KeyCode) -> bool {
    if STATE.with(|s| s.borrow().pending.is_some()) && key != KeyCode::Escape {
        return true;
    }
    match key {
        KeyCode::ArrowLeft | KeyCode::ArrowRight => STATE.with(|s| {
            s.borrow_mut()
                .menu
                .move_family(if key == KeyCode::ArrowLeft { -1 } else { 1 })
        }),
        KeyCode::ArrowUp | KeyCode::ArrowDown => STATE.with(|s| {
            s.borrow_mut()
                .menu
                .move_variant(if key == KeyCode::ArrowUp { -1 } else { 1 })
        }),
        KeyCode::Tab => {
            STATE.with(|s| {
                let mut s = s.borrow_mut();
                s.menu.tab = (s.menu.tab + 1) % 7;
                s.menu.family = 0;
                s.menu.variant = 0;
            });
            ensure(game);
        }
        KeyCode::Enter | KeyCode::NumpadEnter => request(game, game.has_shift_down()),
        KeyCode::KeyT => {} // The vanilla book search is hidden in this screen.
        _ => return false,
    }
    true
}
