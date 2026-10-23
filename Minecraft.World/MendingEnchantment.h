#pragma once
#include "Enchantment.h"

class MendingEnchantment : public Enchantment
{
public:
    MendingEnchantment(int id, int frequency);
    virtual int getMinCost(int level);
    virtual int getMaxCost(int level);
    virtual bool isTreasureOnly() const;
    virtual bool canEnchant(shared_ptr<ItemInstance> item);
    virtual bool isCompatibleWith(Enchantment *other) const;
};
