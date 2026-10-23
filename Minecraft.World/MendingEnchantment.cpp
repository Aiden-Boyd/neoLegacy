#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "MendingEnchantment.h"

MendingEnchantment::MendingEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::all)
{
    setDescriptionId(IDS_ENCHANTMENT_MENDING);
}
int MendingEnchantment::getMinCost(int level) { return 25 * level; }
int MendingEnchantment::getMaxCost(int level) { return getMinCost(level) + 50; }
bool MendingEnchantment::isTreasureOnly() const { return true; }
bool MendingEnchantment::canEnchant(shared_ptr<ItemInstance> item)
{
    return item != nullptr && item->isDamageableItem();
}
bool MendingEnchantment::isCompatibleWith(Enchantment *other) const
{
    return other != Enchantment::arrowInfinite && Enchantment::isCompatibleWith(other);
}
