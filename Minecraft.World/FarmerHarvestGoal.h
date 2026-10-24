#pragma once
#include "Goal.h"
class Villager;
class FarmerHarvestGoal : public Goal
{
    Villager *farmer;
    int targetX,targetY,targetZ,delay,ticks;
    bool active,harvesting;
    bool allowed();
    bool valid(int x,int y,int z,bool harvest);
public:
    explicit FarmerHarvestGoal(Villager *farmer);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
};
