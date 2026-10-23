#include "stdafx.h"
#include "EndGateway.h"
#include "EndOuterFeatures.h"
#include "EndIslandTerrain.h"
#include "ChunkSource.h"
#include "Random.h"
#include "CompoundTag.h"
#include "TileEntityDataPacket.h"
#include "ThrownEnderpearl.h"
#include "LevelData.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.phys.h"
#include <cmath>

EndGatewayTile::EndGatewayTile(int id):BaseEntityTile(id,Material::portal,false) {setLightBlock(0);}
shared_ptr<TileEntity> EndGatewayTile::newTileEntity(Level*) {return std::make_shared<EndGatewayTileEntity>();}
bool EndGatewayTile::isSolidRender(bool) {return false;}
bool EndGatewayTile::isCubeShaped() {return false;}
int EndGatewayTile::getRenderShape() {return SHAPE_INVISIBLE;}
AABB *EndGatewayTile::getAABB(Level*,int,int,int) {return nullptr;}
int EndGatewayTile::getResourceCount(Random*) {return 0;}
int EndGatewayTile::getPistonPushReaction() {return 2;}
int EndGatewayTile::cloneTileId(Level*,int,int,int) {return 0;}
void EndGatewayTile::entityInside(Level *level,int x,int y,int z,shared_ptr<Entity> entity) {
    if(level->isClientSide) return;
    auto gateway=dynamic_pointer_cast<EndGatewayTileEntity>(level->getTileEntity(x,y,z));
    if(gateway) gateway->transport(entity);
}
void EndGatewayTile::animateTick(Level *level,int x,int y,int z,Random *random) {
    for(int i=0;i<3;++i) level->addParticle(eParticleType_ender,x+random->nextDouble(),y+random->nextDouble(),z+random->nextDouble(),
        (random->nextDouble()-.5)*.5,(random->nextDouble()-.5)*.5,(random->nextDouble()-.5)*.5);
}

void EndGatewayTileEntity::load(CompoundTag *tag) {
    TileEntity::load(tag);
    int64_t savedAge=tag->getLong(L"Age");age=savedAge<0?200:savedAge>2147483647?2147483647:int(savedAge);
    cooldown=tag->getInt(L"TeleportCooldown");if(cooldown<0||cooldown>40) cooldown=0;
    linked=tag->getBoolean(L"HasExit");exitX=tag->getInt(L"ExitX");exitY=tag->getInt(L"ExitY");exitZ=tag->getInt(L"ExitZ");
    int edge=END_LEVEL_MIN_WIDTH*8-3;
    if(exitX< -edge||exitX>=edge||exitZ< -edge||exitZ>=edge||exitY<3||exitY>=Level::maxBuildHeight-3) linked=false;
}
void EndGatewayTileEntity::save(CompoundTag *tag) {
    TileEntity::save(tag);tag->putLong(L"Age",age);tag->putInt(L"TeleportCooldown",cooldown);
    tag->putBoolean(L"HasExit",linked);tag->putInt(L"ExitX",exitX);tag->putInt(L"ExitY",exitY);tag->putInt(L"ExitZ",exitZ);
}
shared_ptr<TileEntity> EndGatewayTileEntity::clone() {
    auto copy=std::make_shared<EndGatewayTileEntity>();TileEntity::clone(copy);
    copy->age=age;copy->cooldown=cooldown;copy->linked=linked;copy->exitX=exitX;copy->exitY=exitY;copy->exitZ=exitZ;return copy;
}
double EndGatewayTileEntity::getViewDistance() {return 256.0*256.0;}
shared_ptr<Packet> EndGatewayTileEntity::getUpdatePacket() {
    CompoundTag *tag=new CompoundTag();save(tag);
    return std::make_shared<TileEntityDataPacket>(x,y,z,TileEntityDataPacket::TYPE_END_GATEWAY,tag);
}
void EndGatewayTileEntity::setExit(int xx,int yy,int zz) {exitX=xx;exitY=yy;exitZ=zz;linked=true;setChanged();}
void EndGatewayTileEntity::startCooldown() {
    cooldown=40;
    if(level&&!level->isClientSide) {level->tileEvent(x,y,z,Tile::endGateway_Id,1,40);setChanged();}
}
bool EndGatewayTileEntity::triggerEvent(int type,int value) {
    if(type==1) {cooldown=40;return true;}
    if(type==2) {age=0;return true;}
    return TileEntity::triggerEvent(type,value);
}
void EndGatewayTileEntity::tick() {
    if(age<2147483647) ++age;
    if(cooldown>0) --cooldown;
    else if(level&&!level->isClientSide&&age%2400==0) startCooldown();
    if(level&&!level->isClientSide&&(age==200||cooldown==1)) setChanged();
}
bool EndGatewayTileEntity::transport(shared_ptr<Entity> entity) {
    if(!level||level->isClientSide||level->dimension->id!=1||cooldown>0||age<200||!entity||entity->removed) return false;
    auto pearl=dynamic_pointer_cast<ThrownEnderpearl>(entity);
    shared_ptr<Entity> traveler=pearl?pearl->getOwner():entity;
    if(!traveler||traveler->removed||traveler->level!=level||traveler->riding||traveler->rider.lock()||traveler->GetType()==eTYPE_ENDERDRAGON) return false;
    startCooldown();
    if(!linked&&!EndGateways::createExit(this)) return false;
    // Make the destination's collision/height data available before choosing an exit.
    for(int dx=-1;dx<=1;++dx) for(int dz=-1;dz<=1;++dz) level->getChunkAt(exitX+dx*16,exitZ+dz*16);
    double xx,yy,zz;
    if(!EndGateways::safeExit(level,exitX,exitY,exitZ,traveler,xx,yy,zz)) return false;
    auto destination=dynamic_pointer_cast<EndGatewayTileEntity>(level->getTileEntity(exitX,exitY,exitZ));
    if(destination) destination->startCooldown();
    auto living=dynamic_pointer_cast<LivingEntity>(traveler);
    if(living) living->teleportTo(xx,yy,zz);else traveler->moveTo(xx,yy,zz,traveler->yRot,traveler->xRot);
    traveler->fallDistance=0;
    if(pearl) pearl->remove();
    return true;
}

shared_ptr<EndGatewayTileEntity> EndGateways::placeFrame(Level *level,int x,int y,int z) {
    if(level->isClientSide||level->dimension->id!=1||y<3||y>=Level::maxBuildHeight-3) return nullptr;
    int edge=level->dimension->getXZSize()*8-2;
    if(x< -edge||x>=edge||z< -edge||z>=edge) return nullptr;
    for(int dy=-2;dy<=2;++dy) for(int dx=-1;dx<=1;++dx) for(int dz=-1;dz<=1;++dz) {
        int id=0;
        if(dx==0&&dy==0&&dz==0) id=Tile::endGateway_Id;
        else if((dy==-2||dy==2)&&dx==0&&dz==0) id=Tile::unbreakable_Id;
        else if((dy==-1||dy==1)&&(dx==0||dz==0)) id=Tile::unbreakable_Id;
        level->setTileAndData(x+dx,y+dy,z+dz,id,0,Tile::UPDATE_ALL);
    }
    auto result=dynamic_pointer_cast<EndGatewayTileEntity>(level->getTileEntity(x,y,z));
    if(result) {result->age=0;result->setChanged();level->tileEvent(x,y,z,Tile::endGateway_Id,2,200);}
    return result;
}
void EndGateways::dragonDefeated(Level *level) {
    if(level->isClientSide||level->dimension->id!=1) return;
    int count=level->getLevelData()->getEndGatewaysGenerated();if(count>=20) return;
    int slots[20];for(int i=0;i<20;++i) slots[i]=i;
    Random order(level->getSeed());
    for(int i=19;i>0;--i) {int j=order.nextInt(i+1),v=slots[i];slots[i]=slots[j];slots[j]=v;}
    double angle=slots[count]*6.283185307179586/20.0;
    int x=int(std::floor(std::cos(angle)*96)),z=int(std::floor(std::sin(angle)*96));
    if(placeFrame(level,x,75,z)) level->getLevelData()->setEndGatewaysGenerated(count+1);
}
void EndGateways::migrateDefeatedWorld(Level *level) {
    if(level->isClientSide||level->dimension->id!=1||level->getLevelData()->getEndGatewaysGenerated()!=0) return;
    // An active central exit portal distinguishes a previously defeated dragon
    // from the inactive podium. Only examine already-loaded central chunks.
    if(!level->hasChunksAt(-1,level->seaLevel,-1,1,level->seaLevel,1)) return;
    if(level->getTile(1,level->seaLevel,0)==Tile::endPortalTile_Id||level->getTile(0,level->seaLevel,1)==Tile::endPortalTile_Id) dragonDefeated(level);
}
bool EndGateways::createExit(EndGatewayTileEntity *gateway) {
    Level *level=gateway->level;
    double len=std::sqrt(double(gateway->x)*gateway->x+double(gateway->z)*gateway->z);
    if(len<1) return false;
    int x=int(std::floor(gateway->x/len*1280)),z=int(std::floor(gateway->z/len*1280));
    for(int dx=-1;dx<=1;++dx) for(int dz=-1;dz<=1;++dz) level->getChunkAt(x+dx*16,z+dz*16);
    int bestX=x,bestZ=z,bestY=-1;
    for(int dx=-12;dx<=12;++dx) for(int dz=-12;dz<=12;++dz) {
        int top=level->getHeightmap(x+dx,z+dz);
        for(int y=top-1;y>=40;--y) if(level->getTile(x+dx,y,z+dz)==Tile::endStone_Id) {
            if(y>bestY) {bestY=y;bestX=x+dx;bestZ=z+dz;}break;
        }
    }
    if(bestY<0) {
        Random random(level->getSeed()^int64_t(x)*341873128712LL^int64_t(z)*132897987541LL);
        EndOuterFeatures::placeSmallIsland(level,&random,x,65,z);bestX=x;bestZ=z;bestY=65;
    }
    int centerY=bestY+10;
    auto exit=placeFrame(level,bestX,centerY,bestZ);
    if(!exit) return false;
    exit->setExit(gateway->x,gateway->y,gateway->z);
    gateway->setExit(bestX,centerY,bestZ);exit->startCooldown();return true;
}
bool EndGateways::safeExit(Level *level,int x,int y,int z,shared_ptr<Entity> entity,double &outX,double &outY,double &outZ) {
    // Prefer solid terrain near the destination. Bedrock is excluded from the
    // search so normal returns land on the island instead of the portal frame.
    int best=-1;double bestDistance=1e30;bool found=false;
    auto test=[&](int xx,int yy,int zz) {
        double px=xx+.5,py=yy+1.,pz=zz+.5,half=entity->bbWidth/2.;
        AABB *box=AABB::newTemp(px-half,py,pz-half,px+half,py+entity->bbHeight,pz+half);
        if(!level->getCubes(entity,box)->empty()||level->containsAnyLiquid(box)) return false;
        outX=px;outY=py;outZ=pz;return true;
    };
    for(int dx=-5;dx<=5;++dx) for(int dz=-5;dz<=5;++dz) {
        int top=level->getHeightmap(x+dx,z+dz);
        for(int yy=top-1;yy>=0;--yy) {
            int id=level->getTile(x+dx,yy,z+dz);
            if(id==0||id==Tile::unbreakable_Id||!Tile::tiles[id]||!Tile::tiles[id]->isCubeShaped()||!Tile::tiles[id]->material->blocksMotion()) continue;
            double distance=double(dx)*dx+double(dz)*dz;
            if((yy>best||(yy==best&&distance<bestDistance))&&test(x+dx,yy,z+dz)) {best=yy;bestDistance=distance;found=true;}break;
        }
    }
    if(found) return true;
    // Both generated frames have an indestructible cap and open headroom.
    return level->getTile(x,y+2,z)==Tile::unbreakable_Id&&test(x,y+2,z);
}
void EndGateways::decorateOuter(Level *level,Random *random,int cx,int cz,float height) {
    int half=END_LEVEL_MIN_WIDTH/2;
    if(level->isClientSide||!EndIslandTerrain::isOuterChunk(cx,cz)||cx< -half||cz< -half||cx+1>=half||cz+1>=half||height<=40||random->nextInt(700)!=0) return;
    int x=cx*16+8+random->nextInt(16),z=cz*16+8+random->nextInt(16),top=level->getHeightmap(x,z);
    if(top<=0) return;
    auto gateway=placeFrame(level,x,top+8,z);if(gateway) gateway->setExit(0,level->seaLevel+3,0);
}
