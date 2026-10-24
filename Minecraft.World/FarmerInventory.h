#pragma once
#include <memory>
#include <vector>
class ItemInstance;
class CompoundTag;
// Eight ordinary ItemInstance slots: metadata and custom tags survive save/reload.
class FarmerInventory
{
    std::vector<std::shared_ptr<ItemInstance> > slots;
public:
    FarmerInventory();
    static bool accepts(int id);
    int add(std::shared_ptr<ItemInstance> item);
    int count(int id) const;
    int plantingItem() const;
    bool consume(int id);
    bool wantsFood() const;
    void save(CompoundTag *tag);
    void load(CompoundTag *tag);
};
