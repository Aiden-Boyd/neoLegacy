#include "stdafx.h"
#include "FrostWalkerEnchantment.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "Facing.h"

FrostWalkerEnchantment::FrostWalkerEnchantment(int id, int frequency)
    : Enchantment(id, frequency, EnchantmentCategory::armor_feet)
{
    setDescriptionId(IDS_ENCHANTMENT_FROST_WALKER);
}
int FrostWalkerEnchantment::getMinCost(int level) { return 10 * level; }
int FrostWalkerEnchantment::getMaxCost(int level) { return getMinCost(level) + 15; }
int FrostWalkerEnchantment::getMaxLevel() { return 2; }
bool FrostWalkerEnchantment::isTreasureOnly() const { return true; }
bool FrostWalkerEnchantment::isCompatibleWith(Enchantment *other) const
{
    return other != Enchantment::waterWalker && Enchantment::isCompatibleWith(other);
}
void FrostWalkerEnchantment::freezeNearby(shared_ptr<LivingEntity> entity, Level *level, int x, int y, int z, int enchantmentLevel)
{
    if (level->isClientSide || !entity->onGround || enchantmentLevel <= 0 || y <= 0 || y > Level::maxBuildHeight) return;
    const int radius = min(16, 2 + min(enchantmentLevel, 14));
    const int surface = y - 1;
    for (int xx = x - radius; xx <= x + radius; ++xx)
    for (int zz = z - radius; zz <= z + radius; ++zz)
    {
        const double dx = xx + 0.5 - entity->x, dy = surface + 0.5 - entity->bb->y0, dz = zz + 0.5 - entity->z;
        if (dx * dx + dy * dy + dz * dz > radius * radius || !level->hasChunkAt(xx, surface, zz)) continue;
        const int tile = level->getTile(xx, surface, zz);
        if ((tile != Tile::water_Id && tile != Tile::calmWater_Id) || level->getData(xx, surface, zz) != 0 || !level->isEmptyTile(xx, y, zz)) continue;
        if (!level->mayPlace(Tile::frostedIce_Id, xx, surface, zz, false, Facing::DOWN, nullptr, nullptr)) continue;
        if (level->setTileAndData(xx, surface, zz, Tile::frostedIce_Id, 0, Tile::UPDATE_ALL))
            level->addToTickNextTick(xx, surface, zz, Tile::frostedIce_Id, 60 + entity->getRandom()->nextInt(61));
    }
}
