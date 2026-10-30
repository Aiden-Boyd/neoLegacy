#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"

#include "Sponge.h"

const unsigned int Sponge::SPONGE_NAMES[SPONGE_NAMES_LENGTH] = { IDS_TILE_SPONGE, IDS_TILE_WET_SPONGE };
const unsigned int Sponge::SPONGE_DESCRIPTIONS[SPONGE_NAMES_LENGTH] = { IDS_DESC_SPONGE, IDS_DESC_WET_SPONGE };

Sponge::Sponge(int id) : Tile(id, Material::sponge)
{
	icon = nullptr;
    icon_wet = nullptr;
}

void Sponge::onPlace(Level *level, int x, int y, int z)
{
    tryAbsorb(level, x, y, z);
}

unsigned int Sponge::getDescriptionId(int iData)
{
    if (iData < 0 || iData >= SPONGE_NAMES_LENGTH) iData = 0;

    return SPONGE_DESCRIPTIONS[iData];
}

void Sponge::neighborChanged(Level* level, int x, int y, int z, int type)
{
    tryAbsorb(level, x, y, z);
}

void Sponge::tryAbsorb(Level* level, int x, int y, int z) {
    // Water removal and wet metadata belong to the authoritative world.
    // Client placement/neighbor callbacks wait for the host's block updates.
    if (!level->isClientSide && level->getData(x, y, z) != 1)
        absorb(level, x, y, z);
}

bool Sponge::absorb(Level* level, int x, int y, int z) {
    // The limit is checked after all six neighbors, so a scan starting at
    // 64 can reach 70. Keep that existing behavior and room for the root.
    // Local storage avoids heap allocation even when there is no water.
    static const int MAX_ABSORBED = 64 + 6;
    std::tuple<int, int, int, int> queue[MAX_ABSORBED + 1];
    std::tuple<int, int, int> absorbed[MAX_ABSORBED];
    int head = 0, tail = 0;
    queue[tail++] = { x, y, z, 0 };
    int count = 0;
    int dx[] = { 1,-1,0,0,0,0 };
    int dy[] = { 0,0,1,-1,0,0 };
    int dz[] = { 0,0,0,0,1,-1 };
    while (head < tail)
    {
        auto [bx, by, bz, depth] = queue[head++];
        for (int i = 0; i < 6; i++)
        {
            int nx = bx + dx[i];
            int ny = by + dy[i];
            int nz = bz + dz[i];
            if (level->getMaterial(nx, ny, nz) == Material::water)
            {
                const auto position = std::make_tuple(nx, ny, nz);
                if (std::find(absorbed, absorbed + count, position) != absorbed + count)
                    continue;
                absorbed[count] = position;
                ++count;
                if (depth < 6)
                    queue[tail++] = { nx, ny, nz, depth + 1 };
            }
        }
        if (count > 64)
            break;
    }

    if (count == 0)
        return false;

    // Publish this block's wet state before water removal notifies neighbors.
    // Those callbacks can revisit this sponge or start another sponge's absorption.
    if (!level->setTileAndData(x, y, z, Tile::sponge_Id, 1, UPDATE_CLIENTS))
        return false;

    for (int i = 0; i < count; ++i)
    {
        auto [ax, ay, az] = absorbed[i];
        if (level->getMaterial(ax, ay, az) == Material::water)
            level->removeTile(ax, ay, az);
    }

    return true;
}

void Sponge::animateTick(Level* level, int x, int y, int z, Random* random)
{
    if (level->getData(x, y, z) == 1) {
        int facings[] = { Facing::UP, Facing::DOWN, Facing::WEST, Facing::EAST };
        int facing = facings[rand() % 4];

        if (facing != Facing::UP && !level->isTopSolidBlocking(x + Facing::STEP_X[facing], y + Facing::STEP_Y[facing], z + Facing::STEP_Z[facing]))
        {
            double d0 = (double)x;
            double d1 = (double)y;
            double d2 = (double)z;

            if (facing == Facing::DOWN)
            {
                d1 = d1 - 0.05;
                d0 += (double)rand() / RAND_MAX;
                d2 += (double)rand() / RAND_MAX;
            }
            else
            {
                d1 = d1 + ((double)rand() / RAND_MAX) * 0.8;

                if (facing == Facing::EAST)
                {
                    d2 += (double)rand() / RAND_MAX;
                    ++d0;
                }
                else if (facing == Facing::WEST)
                {
                    d2 += (double)rand() / RAND_MAX;
                    d0 += 0.05;
                }
                else
                {
                    d0 += (double)rand() / RAND_MAX;
                    d2 += 0.05;
                }
            }

            level->addParticle(eParticleType_dripWater, d0, d1, d2, 0.0, 0.0, 0.0);
        }
    }
}

Icon* Sponge::getTexture(int face, int data)
{
    return data == 1 ? icon_wet : icon;
}

void Sponge::registerIcons(IconRegister* iconRegister)
{
    icon_wet = iconRegister->registerIcon(L"sponge_wet");
    icon = iconRegister->registerIcon(L"sponge");
}

int Sponge::getSpawnResourcesAuxValue(int data)
{
    return data;
}
