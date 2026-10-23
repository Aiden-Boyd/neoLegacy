#pragma once
#include "FoodItem.h"

class ChorusFruitItem : public FoodItem
{
public:
    explicit ChorusFruitItem(int id);
    shared_ptr<ItemInstance> use(shared_ptr<ItemInstance>,Level*,shared_ptr<Player>) override;
    shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance>,Level*,shared_ptr<Player>) override;
    static bool teleport(Level*,shared_ptr<Player>);
};
