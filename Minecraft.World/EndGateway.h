#pragma once
#include "BaseEntityTile.h"
#include "TileEntity.h"

class EndGatewayTileEntity : public TileEntity
{
public:
    int age=200,cooldown=0;
    bool linked=false;
    int exitX=0,exitY=75,exitZ=0;
    eINSTANCEOF GetType() override {return eTYPE_ENDGATEWAYTILEENTITY;}
    static TileEntity *create() {return new EndGatewayTileEntity();}
    void load(CompoundTag*) override;
    void save(CompoundTag*) override;
    shared_ptr<TileEntity> clone() override;
    shared_ptr<Packet> getUpdatePacket() override;
    void tick() override;
    bool triggerEvent(int,int) override;
    double getViewDistance() override;
    void setExit(int,int,int);
    void startCooldown();
    bool transport(shared_ptr<Entity>);
};

class EndGatewayTile : public BaseEntityTile
{
public:
    explicit EndGatewayTile(int);
    shared_ptr<TileEntity> newTileEntity(Level*) override;
    bool isSolidRender(bool=false) override;
    bool isCubeShaped() override;
    int getRenderShape() override;
    AABB *getAABB(Level*,int,int,int) override;
    void entityInside(Level*,int,int,int,shared_ptr<Entity>) override;
    int getResourceCount(Random*) override;
    int getPistonPushReaction() override;
    int cloneTileId(Level*,int,int,int) override;
    void animateTick(Level*,int,int,int,Random*) override;
};

class EndGateways
{
public:
    static shared_ptr<EndGatewayTileEntity> placeFrame(Level*,int,int,int);
    static void dragonDefeated(Level*);
    static void migrateDefeatedWorld(Level*);
    static bool createExit(EndGatewayTileEntity*);
    static bool safeExit(Level*,int,int,int,shared_ptr<Entity>,double&,double&,double&);
    static void decorateOuter(Level*,Random*,int,int,float);
};
