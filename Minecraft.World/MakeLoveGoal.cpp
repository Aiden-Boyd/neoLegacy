#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "MakeLoveGoal.h"

MakeLoveGoal::MakeLoveGoal(Villager *villager)
{
	village = weak_ptr<Village>();
	partner = weak_ptr<Villager>();
	loveMakingTime = 0;

	this->villager = villager;
	level = villager->level;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool MakeLoveGoal::canUse()
{

	if (level->isClientSide || !villager->isAlive() || villager->isTrading() || villager->getAge() != 0) return false;
	if (villager->getRandom()->nextInt(500) != 0) return false;

	village = level->villages->getClosestVillage(Mth::floor(villager->x), Mth::floor(villager->y), Mth::floor(villager->z), 0);
	if (village.lock() == nullptr) return false;
	if (!villageNeedsMoreVillagers() || !villager->isWilling(true)) return false;

	shared_ptr<Entity> mate = level->getClosestEntityOfClass(typeid(Villager), villager->bb->grow(8, 3, 8), villager->shared_from_this());
	if (mate == nullptr) return false;

	partner = weak_ptr<Villager>(dynamic_pointer_cast<Villager>(mate));
	if (!partner.lock() || !partner.lock()->isAlive() || partner.lock()->isTrading() || partner.lock()->getAge() != 0 || !partner.lock()->isWilling(true)) return false;

	return true;
}

void MakeLoveGoal::start()
{
	loveMakingTime = 300;
	villager->setInLove(true);
}

void MakeLoveGoal::stop()
{
	village = weak_ptr<Village>();
	partner = weak_ptr<Villager>();
	villager->setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
	auto mate=partner.lock();
    return mate && mate->isAlive() && mate->level==level && !mate->isTrading() && mate->getAge()==0 && mate->isWilling(false) && villager->isAlive() && !villager->isTrading() && loveMakingTime >= 0 && villageNeedsMoreVillagers() && villager->getAge() == 0 && villager->isWilling(false);
}

void MakeLoveGoal::tick()
{
    if(!canContinueToUse())return;
	--loveMakingTime;
	villager->getLookControl()->setLookAt(partner.lock(), 10, 30);

	if (villager->distanceToSqr(partner.lock()) > 1.5 * 1.5)
	{
		villager->getNavigation()->moveTo(partner.lock(), 0.25f);
	}
	else
	{
		if (loveMakingTime == 0 && partner.lock()->isInLove()) breed();
	}

	if (villager->getRandom()->nextInt(35) == 0) level->broadcastEntityEvent(villager->shared_from_this(), EntityEvent::LOVE_HEARTS);
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
	shared_ptr<Village> _village = village.lock();
	if( _village == nullptr ) return false;

	if (!_village->isBreedTimerOk())
	{
		return false;
	}

	int idealSize = static_cast<int>((float)_village->getDoorCount() * 0.35);
	// System.out.println("idealSize: " + idealSize + " pop: " +
	// village.getPopulationSize());
	return _village->getPopulationSize() < idealSize;
}

void MakeLoveGoal::breed()
{
    auto mate=partner.lock();
    if(level->isClientSide||!canContinueToUse()||!mate||!mate->isInLove())return;
    // Keep native capacity limits and avoid consuming willingness on failed spawn.
    if(!level->canCreateMore(eTYPE_VILLAGER,Level::eSpawnType_Breed))return;
    auto child=dynamic_pointer_cast<Villager>(villager->getBreedOffspring(mate));
    if(!child)return;
    child->setAge(-20*60*20);
    child->moveTo(villager->x,villager->y,villager->z,0,0);
    if(!level->addEntity(child))return;
    mate->setAge(5*60*20);villager->setAge(5*60*20);
    mate->setWilling(false);villager->setWilling(false);
    level->broadcastEntityEvent(child,EntityEvent::LOVE_HEARTS);
}
