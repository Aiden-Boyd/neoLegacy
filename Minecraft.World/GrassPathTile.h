#pragma once
#include "Tile.h"
class GrassPathTile : public Tile
{
    Icon *side;
public:
    GrassPathTile(int id);
    virtual void updateDefaultShape();
    virtual AABB *getAABB(Level *level,int x,int y,int z);
    virtual bool isSolidRender(bool isServerLevel=false);
    virtual bool isCubeShaped();
    virtual bool shouldRenderFace(LevelSource *level,int x,int y,int z,int face);
    virtual Icon *getTexture(int face,int data);
    virtual void registerIcons(IconRegister *icons);
    virtual int getResource(int data,Random *random,int fortune);
    virtual bool isSilkTouchable();
    virtual void onPlace(Level *level,int x,int y,int z);
    virtual void neighborChanged(Level *level,int x,int y,int z,int type);
};
