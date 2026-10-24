# Villager food and breeding candidate

Cumulative base: c734730047586793277ff60994d38d1f745cf954.

All adult professions can collect the seven existing farm item types when
mobGriefing permits pickup. Farming remains farmer-only. Nearby adults with
surplus food can share after a ten-tick delay; recipients must need more food,
be within three blocks, visible, alive and not trading. The social check has a
1-in-50 per-tick chance and uses a bounded nearby entity query. Sharing throws a
real item with ten ticks of pickup delay; it is not an invisible inventory transfer.
Spawn failure retains the donor's items. Client code never spawns gifts.

Bread/carrot/potato/beetroot stacks share half. Farmers can convert half a wheat
stack rounded down to complete groups of three into bread when sharing. They
retain the other wheat. Seeds are not shared. Surplus thresholds are six bread,
24 vegetables or (farmers) 18 wheat; farmers want five breeding rations, other
professions one. Thresholds are per slot, matching the 1.12.2 reference.

Willingness costs three bread or twelve carrots/potatoes/beetroots once, then
persists until successful birth. Wheat and seeds are not breeding food. Trade
restocks also grant willingness. Willing is saved alongside the existing eight
NBT inventory slots. Existing AgableMob Age persistence retains the parent
6000-tick cooldown and child -24000 age. Mid-courtship reload restarts courtship,
retaining willingness rather than consuming food again.

Breeding uses the native 300-tick courtship, door population limit (floor of
0.35 times door count), village no-breed timer and console entity cap. Both
parents must remain alive, adult, willing and not trading in the same level.
Successful child spawn sets cooldowns and clears both willingness flags; null
child creation or rejected spawning does neither. The next attempt uses normal
AI scheduling. Partner age checks prevent both goals from spawning duplicates.

## Automated validation

Production FarmerInventory.cpp, MakeLoveGoal.cpp and Villager food methods are
compiled in tests/villager_food_regression.py. Checks cover ration boundaries,
willingness save/load, sharing conservation/metadata, wheat conversion, delayed
physical gifts, non-farmer pickup, wall/trade/client/dead guards, failed gift and
child spawn, null offspring, village gates, parent cooldown and child age.
Prior Overworld regressions also run. These fixtures are not full pathfinding,
chunk save integration or actual game simulation.

## In-game acceptance

1. Give adult non-farmers bread/carrot/potato/beetroot. Check pickup, blocked by
   mobGriefing=false. Recheck that only farmers harvest and replant.
2. Give a farmer wheat in a real village. Watch it throw bread to an underfed
   nearby adult; verify seed retention, no duplication and no gifts through walls.
3. Test each breeding food in a village with sufficient recognized doors. Check
   two willing adults approach, hearts and one baby. Test no doors/full village,
   unavailable entity capacity, parents trading, and interrupted courtship.
4. Check parents wait five minutes and babies grow in twenty game minutes.
   Reload during courtship/cooldown and verify inventories, ages and willingness.
5. Repeat with two players through Steam/Proton/controller. Look for duplicate
   gifts/babies, stuck trading menus, poor navigation and farmer seed starvation.

Reference: WangTingZheng/mcp940 d0c030a4139ce7cf3f284b180f0d9ea87bdf8141,
EntityAIVillagerInteract, EntityAIVillagerMate and EntityVillager. This candidate
also allows beetroot to pay the twelve-vegetable ration; the examined Java
method checks beetroot for food sufficiency but omits it from consumption.
Exact CU59 social timing/animation parity and in-game acceptance remain pending.
