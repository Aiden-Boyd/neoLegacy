#include "stdafx.h"
#include "FrostedIceTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "IconRegister.h"
#include "Facing.h"

FrostedIceTile::FrostedIceTile(int id) : IceTile(id)
{
    for (int i = 0; i < 4; ++i) stages[i] = nullptr;
}
void FrostedIceTile::registerIcons(IconRegister *icons)
{
    const wchar_t *names[] = {L"frosted_ice_0", L"frosted_ice_1", L"frosted_ice_2", L"frosted_ice_3"};
    for (int i = 0; i < 4; ++i) stages[i] = icons->registerIcon(names[i]);
    icon = stages[0];
}
Icon *FrostedIceTile::getTexture(int face, int data) { return stages[max(0, min(data, 3))]; }
bool FrostedIceTile::isSilkTouchable() { return false; }
int FrostedIceTile::cloneTileId(Level *level, int x, int y, int z) { return 0; }
bool FrostedIceTile::shouldTileTick(Level *level, int x, int y, int z) { return !level->isClientSide; }
int FrostedIceTile::countNeighbors(Level *level, int x, int y, int z)
{
    int count = 0;
    for (int face = 0; face < 6; ++face)
        if (level->getTile(x + Facing::STEP_X[face], y + Facing::STEP_Y[face], z + Facing::STEP_Z[face]) == id && ++count >= 4) return count;
    return count;
}
void FrostedIceTile::melt(Level *level, int x, int y, int z)
{
    if (level->dimension->ultraWarm) level->removeTile(x, y, z);
    else level->setTileAndUpdate(x, y, z, Tile::calmWater_Id);
}
void FrostedIceTile::age(Level *level, int x, int y, int z, Random *random, bool affectNeighbors)
{
    if (level->getTile(x, y, z) != id) return;
    const int current = max(0, min(level->getData(x, y, z), 3));
    if (current < 3)
    {
        if (level->setData(x, y, z, current + 1, Tile::UPDATE_CLIENTS))
            level->addToTickNextTick(x, y, z, id, 20 + random->nextInt(21));
        return;
    }
    melt(level, x, y, z);
    if (affectNeighbors)
        for (int face = 0; face < 6; ++face)
            age(level, x + Facing::STEP_X[face], y + Facing::STEP_Y[face], z + Facing::STEP_Z[face], random, false);
}
void FrostedIceTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (level->isClientSide || level->getTile(x, y, z) != id) return;
    const int current = max(0, min(level->getData(x, y, z), 3));
    if ((random->nextInt(3) == 0 || countNeighbors(level, x, y, z) < 4) &&
        level->getRawBrightness(x, y, z) > 11 - current - Tile::lightBlock[id])
        age(level, x, y, z, random, true);
    else level->addToTickNextTick(x, y, z, id, 20 + random->nextInt(21));
}
void FrostedIceTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    if (!level->isClientSide && type == id && level->getTile(x, y, z) == id && countNeighbors(level, x, y, z) < 2)
        melt(level, x, y, z);
}
