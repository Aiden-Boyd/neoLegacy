#include "stdafx.h"
#include "net.minecraft.h"
#include "EndBuildingTiles.h"

PurpurPillarTile::PurpurPillarTile(int id) : RotatedPillarTile(id, Material::stone)
{
    iconTop = nullptr;
}

void PurpurPillarTile::registerIcons(IconRegister *iconRegister)
{
    icon = iconRegister->registerIcon(L"purpur_pillar");
    iconTop = iconRegister->registerIcon(L"purpur_pillar_top");
}

Icon *PurpurPillarTile::getTypeTexture(int type)
{
    return icon;
}

PurpurSlabTile::PurpurSlabTile(int id, bool fullSize)
    : HalfSlabTile(id, Material::stone), fullSize(fullSize)
{
    DerivedInit();
}

int PurpurSlabTile::isFullSize()
{
    return fullSize ? 1 : 0;
}

int PurpurSlabTile::getResource(int data, Random *random, int playerBonusLevel)
{
    return Tile::purpurSlab_Id;
}

unsigned int PurpurSlabTile::getDescriptionId(int data)
{
    return IDS_TILE_PURPUR_SLAB;
}

int PurpurSlabTile::getAuxName(int auxValue)
{
    return IDS_TILE_PURPUR_SLAB;
}
