#pragma once
#include "CropTile.h"
class BeetrootTile : public CropTile
{
    Icon *stages[4];
public:
    BeetrootTile(int id);
    virtual int getMaxAge();
    virtual void tick(Level *level,int x,int y,int z,Random *random);
    virtual Icon *getTexture(int face,int data);
    virtual void registerIcons(IconRegister *icons);
    virtual void updateDefaultShape();
    virtual void updateShape(LevelSource *level,int x,int y,int z,int forceData=-1,shared_ptr<TileEntity> entity=nullptr);
    virtual void spawnResources(Level *level,int x,int y,int z,int data,float odds,int fortune);
protected:
    virtual int getBonemealAgeIncrease(Level *level);
    virtual int getBaseSeedId();
    virtual int getBasePlantId();
};
