// Original, engine-independent menu model and craft acknowledgement guard.
use std::collections::BTreeMap;
pub const WIDTH: f32 = 300.0;
pub const HEIGHT: f32 = 232.0;
pub const TABS: [&str; 7] = [
    "Structures",
    "Tools",
    "Food",
    "Armour",
    "Mechanisms",
    "Transport",
    "Decoration",
];
#[derive(Clone, Debug, Default)]
pub struct Recipe {
    pub id: String,
    pub book: i32,
    pub shaped: bool,
    pub width: usize,
    pub height: usize,
    pub category: String,
    pub group: String,
    pub output: String,
    pub name: String,
    pub count: u32,
    pub ingredients: Vec<Vec<String>>,
    pub food: bool,
}
impl Recipe {
    pub fn fits(&self, size: usize) -> bool {
        if self.shaped {
            self.width <= size && self.height <= size
        } else {
            self.ingredients.len() <= size * size
        }
    }
    pub fn tab(&self) -> usize {
        let id = self.output.as_str();
        if ["minecart", "rail", "boat", "raft"]
            .iter()
            .any(|s| id.ends_with(s))
        {
            5
        } else if self.food {
            2
        } else if ["helmet", "chestplate", "leggings", "boots"]
            .iter()
            .any(|s| id.ends_with(s))
        {
            3
        } else if self.category.contains("equipment") {
            1
        } else if self.category.contains("redstone") {
            4
        } else if self.category.contains("building") {
            0
        } else {
            6
        }
    }
    pub fn family(&self) -> String {
        for s in [
            "pickaxe",
            "shovel",
            "sword",
            "axe",
            "hoe",
            "helmet",
            "chestplate",
            "leggings",
            "boots",
            "planks",
            "stairs",
            "slab",
            "wool",
            "carpet",
            "boat",
            "door",
            "fence",
        ] {
            if self.output.ends_with(&format!("_{s}")) {
                return format!("family:{s}");
            }
        }
        if self.group.is_empty() {
            self.output.clone()
        } else {
            format!("group:{}", self.group)
        }
    }
}
#[derive(Default, Debug, Clone)]
pub struct Menu {
    pub tab: usize,
    pub family: usize,
    pub variant: usize,
    pub groups: Vec<Vec<usize>>,
}
impl Menu {
    pub fn rebuild(&mut self, recipes: &[Recipe], size: usize) {
        let mut groups: BTreeMap<String, Vec<usize>> = BTreeMap::new();
        for (i, r) in recipes.iter().enumerate() {
            if r.fits(size) && r.tab() == self.tab && r.book >= 0 {
                groups.entry(r.family()).or_default().push(i);
            }
        }
        self.groups = groups.into_values().collect();
        self.family = self.family.min(self.groups.len().saturating_sub(1));
        self.variant = self.variant.min(
            self.groups
                .get(self.family)
                .map_or(0, |g| g.len().saturating_sub(1)),
        );
    }
    pub fn selected(&self) -> Option<usize> {
        self.groups.get(self.family)?.get(self.variant).copied()
    }
    pub fn move_family(&mut self, step: isize) {
        if !self.groups.is_empty() {
            self.family =
                (self.family as isize + step).rem_euclid(self.groups.len() as isize) as usize;
            self.variant = 0;
        }
    }
    pub fn move_variant(&mut self, step: isize) {
        if let Some(g) = self.groups.get(self.family) {
            self.variant = (self.variant as isize + step).rem_euclid(g.len() as isize) as usize;
        }
    }
}
/// Solve ingredient alternatives without double-counting a stack or making greedy tag choices.
pub fn can_craft(recipe: &Recipe, available: &BTreeMap<String, u32>) -> bool {
    fn solve(slots: &[&Vec<String>], stock: &mut BTreeMap<String, u32>) -> bool {
        let Some((first, rest)) = slots.split_first() else {
            return true;
        };
        for id in first.iter() {
            let n = stock.get(id).copied().unwrap_or(0);
            if n == 0 {
                continue;
            }
            stock.insert(id.clone(), n - 1);
            if solve(rest, stock) {
                return true;
            }
            stock.insert(id.clone(), n);
        }
        false
    }
    let mut slots: Vec<_> = recipe
        .ingredients
        .iter()
        .filter(|s| !s.is_empty())
        .collect();
    slots.sort_by_key(|s| s.len());
    solve(&slots, &mut available.clone())
}
#[derive(Clone, Debug)]
pub struct Pending {
    pub window: i32,
    pub recipe: usize,
    pub all: bool,
    pub ack: u64,
    pub started_ms: u64,
}
#[derive(Debug, PartialEq)]
pub enum Decision {
    Wait,
    Take,
    Cancel,
}
impl Pending {
    pub fn poll(
        &self,
        window: i32,
        now_ms: u64,
        ack: u64,
        ready: bool,
        cursor_empty: bool,
    ) -> Decision {
        if self.window != window || !cursor_empty || now_ms.saturating_sub(self.started_ms) >= 5000
        {
            Decision::Cancel
        } else if ack != self.ack && ready {
            Decision::Take
        } else {
            Decision::Wait
        }
    }
}
pub fn toolbar_hit(x: f32, y: f32) -> bool {
    (8.0..295.0).contains(&x) && (17.0..62.0).contains(&y)
        || (19.0..42.0).contains(&x) && (75.0..135.0).contains(&y)
        || (218.0..290.0).contains(&x)
            && ((110.0..130.0).contains(&y)
                || (134.0..154.0).contains(&y)
                || (185.0..205.0).contains(&y))
}
pub fn slot_position(slot: usize, size: usize) -> Option<(f32, f32)> {
    match slot {
        0..9 => Some((8.0 + slot as f32 * 18.0, 210.0)),
        9..36 => Some((
            8.0 + ((slot - 9) % 9) as f32 * 18.0,
            150.0 + ((slot - 9) / 9) as f32 * 18.0,
        )),
        100..109 if slot - 100 < size * size => Some((
            104.0 + ((slot - 100) % size) as f32 * 18.0,
            77.0 + ((slot - 100) / size) as f32 * 18.0,
        )),
        200 => Some((194.0, 95.0)),
        500..504 => Some((190.0, 150.0 + (slot - 500) as f32 * 18.0)),
        504 => Some((218.0, 164.0)),
        _ => None,
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    fn r() -> Recipe {
        Recipe {
            book: 1,
            output: "minecraft:oak_planks".into(),
            category: "crafting_building_blocks".into(),
            ..Recipe::default()
        }
    }
    #[test]
    fn alternatives_do_not_double_count() {
        let mut r = r();
        r.ingredients = vec![vec!["oak".into()], vec!["oak".into()]];
        assert!(!can_craft(&r, &BTreeMap::from([("oak".into(), 1)])));
        assert!(can_craft(&r, &BTreeMap::from([("oak".into(), 2)])));
    }
    #[test]
    fn tag_choices_backtrack() {
        let mut r = r();
        r.ingredients = vec![vec!["oak".into(), "birch".into()], vec!["oak".into()]];
        assert!(can_craft(
            &r,
            &BTreeMap::from([("oak".into(), 1), ("birch".into(), 1)])
        ));
    }
    #[test]
    fn shaped_and_shapeless_respect_grid() {
        let mut r = r();
        r.shaped = true;
        r.width = 3;
        r.height = 1;
        assert!(!r.fits(2));
        assert!(r.fits(3));
        r.shaped = false;
        r.ingredients = vec![vec![]; 5];
        assert!(!r.fits(2));
    }
    #[test]
    fn variants_group_and_empty_selection_is_safe() {
        let mut b = r();
        b.output = "minecraft:birch_planks".into();
        let mut m = Menu::default();
        m.rebuild(&[r(), b], 2);
        assert_eq!(m.groups.len(), 1);
        m.move_variant(-1);
        assert_eq!(m.variant, 1);
        m.rebuild(&[], 2);
        assert_eq!(m.selected(), None);
        m.move_family(1);
        m.move_variant(1);
    }
    #[test]
    fn transport_and_armour_have_separate_tabs() {
        let mut r = r();
        r.output = "minecraft:iron_helmet".into();
        assert_eq!(r.tab(), 3);
        r.output = "minecraft:oak_boat".into();
        assert_eq!(r.tab(), 5);
    }
    fn p() -> Pending {
        Pending {
            window: 1,
            recipe: 0,
            all: false,
            ack: 7,
            started_ms: 0,
        }
    }
    #[test]
    fn no_collection_before_server_ack() {
        assert_eq!(p().poll(1, 10, 7, true, true), Decision::Wait);
        assert_eq!(p().poll(1, 10, 8, false, true), Decision::Wait);
        assert_eq!(p().poll(1, 10, 8, true, true), Decision::Take);
    }
    #[test]
    fn timeout_menu_changes_and_cursor_cancel() {
        assert_eq!(p().poll(1, 5000, 8, true, true), Decision::Cancel);
        assert_eq!(p().poll(2, 10, 8, true, true), Decision::Cancel);
        assert_eq!(p().poll(1, 10, 8, true, false), Decision::Cancel);
    }
    #[test]
    fn toolbar_never_captures_outside_or_inventory_slots() {
        assert!(!toolbar_hit(-1.0, 30.0));
        assert!(!toolbar_hit(500.0, 120.0));
        assert!(!toolbar_hit(8.0, 150.0));
        assert!(toolbar_hit(220.0, 120.0));
        assert!(toolbar_hit(25.0, 90.0));
    }
    #[test]
    fn every_slot_fits_its_window() {
        for size in [2, 3] {
            for slot in (0..36)
                .chain(100..100 + size * size)
                .chain([200])
                .chain(500..505)
            {
                let (x, y) = slot_position(slot, size).unwrap();
                assert!(x >= 0.0 && y >= 0.0 && x + 16.0 <= WIDTH && y + 16.0 <= HEIGHT);
            }
        }
    }
}
