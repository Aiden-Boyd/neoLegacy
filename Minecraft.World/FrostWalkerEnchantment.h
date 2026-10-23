#pragma once
#include "Enchantment.h"
class Level;
class FrostWalkerEnchantment : public Enchantment
{
public:
    FrostWalkerEnchantment(int id, int frequency);
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual int getMaxLevel();
    virtual bool isTreasureOnly() const;
    virtual bool isCompatibleWith(Enchantment *other) const;
    static void freezeNearby(shared_ptr<LivingEntity> entity, Level *level, int x, int y, int z, int enchantmentLevel);
};
