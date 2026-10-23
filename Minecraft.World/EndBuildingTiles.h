#pragma once

#include "HalfSlabTile.h"
#include "RotatedPillarTile.h"

class PurpurPillarTile : public RotatedPillarTile
{
public:
    explicit PurpurPillarTile(int id);
    void registerIcons(IconRegister *iconRegister) override;

protected:
    Icon *getTypeTexture(int type) override;
};

// Use the existing slab geometry and StoneSlabTileItem merging path.
// A separate ID pair keeps purpur out of the red sandstone slab metadata.
class PurpurSlabTile : public HalfSlabTile
{
    bool fullSize;

public:
    PurpurSlabTile(int id, bool fullSize);
    int isFullSize() override;
    int getResource(int data, Random *random, int playerBonusLevel) override;
    unsigned int getDescriptionId(int data = -1) override;
    int getAuxName(int auxValue) override;
};
