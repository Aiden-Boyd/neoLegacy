#pragma once
#include "Tile.h"
#include "RotatedPillarTile.h"
class MagmaTile : public Tile
{
public:
    MagmaTile(int id);
    virtual int getLightColor(LevelSource *level,int x,int y,int z,int tileId=-1);
    virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
    virtual void tick(Level *level, int x, int y, int z, Random *random);
};
class BoneBlockTile : public RotatedPillarTile
{
public:
    BoneBlockTile(int id);
    virtual void registerIcons(IconRegister *icons);
    virtual int getSpawnResourcesAuxValue(int data);
    virtual int getType(int data);
protected:
    virtual Icon *getTypeTexture(int type);
};
