#pragma once
#include "IceTile.h"
class FrostedIceTile : public IceTile
{
    Icon *stages[4];
    int countNeighbors(Level *level, int x, int y, int z);
    void melt(Level *level, int x, int y, int z);
    void age(Level *level, int x, int y, int z, Random *random, bool affectNeighbors);
public:
    FrostedIceTile(int id);
    virtual void registerIcons(IconRegister *icons);
    virtual Icon *getTexture(int face, int data);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual bool shouldTileTick(Level *level, int x, int y, int z);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual bool isSilkTouchable();
    virtual int cloneTileId(Level *level, int x, int y, int z);
};
