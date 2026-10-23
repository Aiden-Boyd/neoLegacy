#pragma once
#include "Tile.h"

struct EndPlantUV { float u0, v0, u1, v1; int texture; };
struct EndPlantPart
{
    float x0,y0,z0,x1,y1,z1;
    unsigned int faces;
    int facing;
    EndPlantUV uv[6];
};

// Model-space quarter turns shared by the renderer and geometry tests.
void rotateEndPlantPoint(int facing, float &x, float &y, float &z);

class EndPlantTile : public Tile
{
public:
    static const int MAX_PARTS = 12;
    EndPlantTile(int id, Material *material);
    bool isSolidRender(bool isServerLevel = false) override;
    bool isCubeShaped() override;
    bool isPathfindable(LevelSource *level,int x,int y,int z) override;
    int getRenderShape() override;
    virtual int getParts(LevelSource *level,int x,int y,int z,int data,EndPlantPart *parts) = 0;
    virtual Icon *getPartTexture(int texture,int data);
    AABB *getAABB(Level *level,int x,int y,int z) override;
    AABB *getTileAABB(Level *level,int x,int y,int z) override;
    bool isSilkTouchable() override;
};

class EndRodTile : public EndPlantTile
{
public:
    explicit EndRodTile(int id);
    int getParts(LevelSource*,int,int,int,int,EndPlantPart*) override;
    void updateDefaultShape() override;
    void updateShape(LevelSource*,int,int,int,int = -1,shared_ptr<TileEntity> = {}) override;
    int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int) override;
    void animateTick(Level*,int,int,int,Random*) override;
    int getSpawnResourcesAuxValue(int data) override;
};

class ChorusPlantTile : public EndPlantTile
{
public:
    explicit ChorusPlantTile(int id);
    static bool connects(LevelSource*,int,int,int,int);
    int getParts(LevelSource*,int,int,int,int,EndPlantPart*) override;
    void updateDefaultShape() override;
    void updateShape(LevelSource*,int,int,int,int = -1,shared_ptr<TileEntity> = {}) override;
    void addAABBs(Level*,int,int,int,AABB*,AABBList*,shared_ptr<Entity>) override;
    bool canSurvive(Level*,int,int,int) override;
    bool mayPlace(Level*,int,int,int) override;
    void neighborChanged(Level*,int,int,int,int) override;
    void tick(Level*,int,int,int,Random*) override;
    int getResource(int,Random*,int) override;
    int getResourceCount(Random*) override;
};

class ChorusFlowerTile : public EndPlantTile
{
    Icon *plantIcon, *deadIcon;
public:
    explicit ChorusFlowerTile(int id);
    int getParts(LevelSource*,int,int,int,int,EndPlantPart*) override;
    void registerIcons(IconRegister*) override;
    Icon *getPartTexture(int,int) override;
    Icon *getTexture(int,int) override;
    bool canSurvive(Level*,int,int,int) override;
    bool mayPlace(Level*,int,int,int) override;
    void neighborChanged(Level*,int,int,int,int) override;
    void tick(Level*,int,int,int,Random*) override;
    int getResource(int,Random*,int) override;
    int getSpawnResourcesAuxValue(int) override;
    void playerDestroy(Level*,shared_ptr<Player>,int,int,int,int) override;
    static bool clearNeighbors(Level*,int,int,int,int exceptFace = -1);
};
