# Mending candidate

Target: final Xbox One CU59 content, using the existing Windows64 Steam/Proton runtime. This candidate adds Mending to the previous shulker-box polish build. No existing downloadable candidate is replaced.

## Implemented

- Enchantment ID 70, level I, rare weight 2; localized name and normal enchanted-book tooltip/Creative registration.
- Applies to damageable items including Elytra. Anvils apply books, combine Mending equipment and combine enchanted books; Mending and Infinity are mutually exclusive in either merge order.
- Server XP pickup randomly chooses among the held item and worn armor with Mending. Ordinary inventory contents do not qualify. Empty stacks and non-damageable items are ignored. Equipment aliases are not counted twice.
- Old-version selection occurs before checking damage: selecting an undamaged piece awards XP normally even if another eligible piece is damaged. One piece per orb; 2 durability per XP, integer-floor repair cost, leftover XP awarded once. Client pickup, throw delay and pickup cooldown cannot repair equipment.
- Treasure flag flows through enchantment selection. Default is false, preserving table and ordinary generated-equipment exclusion. Fishing treasure and End-city enchanted loot explicitly enable it. Mending eligibility costs 25–75; other enchantments retain their existing rules.
- Existing random enchanted-book chest generation and librarian trades include the new registry entry. Treasure trades double the generated book price, capped at 64 emeralds. Existing offers and existing loot are not regenerated.

The equipment model currently exposes one held item and four armor slots. This change does not add an offhand system.

## Verification

`python tests/mending_regression.py` compiles the production enchantment classes, equipment selection, available-enchantment pool, XP pickup and anvil compatibility block against a focused fixture. It covers armor/held eligibility, empty/full/odd repairs, XP leftovers, authority/cooldowns, treasure/table exclusion over power values 1–100, book applicability and both Infinity merge directions. Source integration checks verify registration and treasure routing. The existing city fixture additionally asserts that city enchantment generation enables treasure.

Full Windows64 compilation and all existing regression jobs run in Actions. These checks do not establish in-game acceptance or complete CU59 parity. Java 1.12.2 behavior was used as a cross-reference for XP arithmetic, treasure costs and Infinity incompatibility; exact Xbox behavior still needs play testing.

## In-game acceptance

Back up your save and use a test copy.

1. Find the Mending I book in Creative and confirm its tooltip. Apply it in Survival with an anvil to a damaged pickaxe, bow, armor piece and Elytra; check costs and persistence after save/reload.
2. Hold the damaged pickaxe and collect XP from mobs, bottles and a furnace. Confirm repair and normal leftover XP; repeat wearing damaged Elytra and armor.
3. Put Mending items in ordinary inventory and collect XP while holding an unenchanted item. Only worn equipment should repair.
4. Wear several Mending pieces, including an undamaged one. Check that repairs vary and some orbs award XP normally. An orb must not repair every piece.
5. Try Mending + Infinity on a bow and on enchanted books, in both anvil input orders. The incompatible enchantment must not be added. Unbreaking remains compatible.
6. Enchant ordinary books/tools at a table; Mending should never be offered. Test fishing treasure, newly generated End-city chests and new librarian offers over multiple rolls; existing loot/offers are unchanged.
7. In multiplayer, have the guest collect XP and reconnect. Confirm the repaired durability synchronizes, persists, and the host receives no duplicate XP.
8. Launch through the existing Steam/Proton entry and navigate Creative/anvil screens with the controller. Native controller UI was not changed.
