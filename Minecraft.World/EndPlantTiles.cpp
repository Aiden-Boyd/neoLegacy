#include "stdafx.h"
#include "EndPlantTiles.h"
#include "EndPlantModels.h"
#include "Facing.h"
#include "IconRegister.h"
#include "Item.h"
#include "ItemInstance.h"
#include "Random.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include <algorithm>

void rotateEndPlantPoint(int face,float &x,float &y,float &z)
{
    float a=x,b=y,c=z;
    switch(face) {
    case Facing::DOWN: y=16-b;z=16-c;break;
    case Facing::NORTH: y=c;z=16-b;break;
    case Facing::SOUTH: y=16-c;z=b;break;
    case Facing::WEST: x=16-b;y=a;break;
    case Facing::EAST: x=b;y=16-a;break;
    default: break;
    }
}

EndPlantTile::EndPlantTile(int id,Material *material):Tile(id,material,false) { setLightBlock(0); }
bool EndPlantTile::isSolidRender(bool) { return false; }
bool EndPlantTile::isCubeShaped() { return false; }
bool EndPlantTile::isPathfindable(LevelSource*,int,int,int) { return false; }
int EndPlantTile::getRenderShape() { return SHAPE_END_PLANT; }
bool EndPlantTile::isSilkTouchable() { return false; }
Icon *EndPlantTile::getPartTexture(int,int) { return icon; }
AABB *EndPlantTile::getAABB(Level *level,int x,int y,int z) {
    updateShape(level,x,y,z); return Tile::getAABB(level,x,y,z);
}
AABB *EndPlantTile::getTileAABB(Level *level,int x,int y,int z) {
    updateShape(level,x,y,z); return Tile::getTileAABB(level,x,y,z);
}

EndRodTile::EndRodTile(int id):EndPlantTile(id,Material::decoration) { updateDefaultShape(); }
void EndRodTile::updateDefaultShape() { setShape(6/16.f,0,6/16.f,10/16.f,1,10/16.f); }
void EndRodTile::updateShape(LevelSource *level,int x,int y,int z,int forceData,shared_ptr<TileEntity>) {
    int face=forceData>=0?forceData:level->getData(x,y,z);
    if(face==Facing::WEST||face==Facing::EAST) setShape(0,6/16.f,6/16.f,1,10/16.f,10/16.f);
    else if(face==Facing::NORTH||face==Facing::SOUTH) setShape(6/16.f,6/16.f,0,10/16.f,10/16.f,1);
    else updateDefaultShape();
}
int EndRodTile::getParts(LevelSource*,int,int,int,int data,EndPlantPart *parts) {
    for(int i=0;i<2;++i) { parts[i]=END_ROD_PARTS[i];parts[i].facing=data>=0&&data<6?data:Facing::UP; }
    return 2;
}
int EndRodTile::getPlacedOnFaceDataValue(Level *level,int x,int y,int z,int face,float,float,float,int) {
    if(face<0||face>=6) face=Facing::UP;
    int bx=x-Facing::STEP_X[face],by=y-Facing::STEP_Y[face],bz=z-Facing::STEP_Z[face];
    if(level->getTile(bx,by,bz)==id && level->getData(bx,by,bz)==face) return Facing::getOpposite(face);
    return face;
}
int EndRodTile::getSpawnResourcesAuxValue(int) { return 0; }
void EndRodTile::animateTick(Level *level,int x,int y,int z,Random *random) {
    int face=level->getData(x,y,z);
    if(face<0||face>=6) face=Facing::UP;
    double along=random->nextFloat();
    double px=x+.5+(random->nextFloat()-.5)*.1+Facing::STEP_X[face]*(along-.5);
    double py=y+.5+(random->nextFloat()-.5)*.1+Facing::STEP_Y[face]*(along-.5);
    double pz=z+.5+(random->nextFloat()-.5)*.1+Facing::STEP_Z[face]*(along-.5);
    if(random->nextInt(5)==0) level->addParticle(eParticleType_endRod,px,py,pz,
        random->nextGaussian()*.005,random->nextGaussian()*.005,random->nextGaussian()*.005);
}

ChorusPlantTile::ChorusPlantTile(int id):EndPlantTile(id,Material::plant) { updateDefaultShape(); }
void ChorusPlantTile::updateDefaultShape() { setShape(3/16.f,3/16.f,3/16.f,13/16.f,13/16.f,13/16.f); }
bool ChorusPlantTile::connects(LevelSource *level,int x,int y,int z,int face) {
    int tile=level->getTile(x+Facing::STEP_X[face],y+Facing::STEP_Y[face],z+Facing::STEP_Z[face]);
    return tile==Tile::chorusPlant_Id||tile==Tile::chorusFlower_Id||(face==Facing::DOWN&&tile==Tile::endStone_Id);
}
int ChorusPlantTile::getParts(LevelSource *level,int x,int y,int z,int,EndPlantPart *parts) {
    if(!level) { for(int i=0;i<7;++i) parts[i]=CHORUS_INVENTORY_PARTS[i];return 7; }
    parts[0]=makeEndPlantBox(4,4,4,12,12,12);
    int count=1;
    for(int face=0;face<6;++face) if(connects(level,x,y,z,face)) {
        parts[0].faces &= ~(1u<<face);
        parts[count]=makeEndPlantBox(4,12,4,12,16,12);
        parts[count].faces &= ~((1u<<Facing::UP)|(1u<<Facing::DOWN));
        parts[count++].facing=face;
    }
    return count;
}
void ChorusPlantTile::updateShape(LevelSource *level,int x,int y,int z,int,shared_ptr<TileEntity>) {
    setShape(connects(level,x,y,z,Facing::WEST)?0:3/16.f,
             connects(level,x,y,z,Facing::DOWN)?0:3/16.f,
             connects(level,x,y,z,Facing::NORTH)?0:3/16.f,
             connects(level,x,y,z,Facing::EAST)?1:13/16.f,
             connects(level,x,y,z,Facing::UP)?1:13/16.f,
             connects(level,x,y,z,Facing::SOUTH)?1:13/16.f);
}
void ChorusPlantTile::addAABBs(Level *level,int x,int y,int z,AABB *box,AABBList *boxes,shared_ptr<Entity>) {
    auto add=[&](float a,float b,float c,float d,float e,float f) {
        AABB *part=AABB::newTemp(x+a,y+b,z+c,x+d,y+e,z+f);
        if(box->intersects(part)) boxes->push_back(part);
    };
    const float lo=3/16.f,hi=13/16.f;
    add(lo,lo,lo,hi,hi,hi);
    for(int face=0;face<6;++face) if(connects(level,x,y,z,face)) {
        float a=lo,b=lo,c=lo,d=hi,e=hi,f=hi;
        if(face==Facing::DOWN){b=0;e=lo;}if(face==Facing::UP){b=hi;e=1;}
        if(face==Facing::NORTH){c=0;f=lo;}if(face==Facing::SOUTH){c=hi;f=1;}
        if(face==Facing::WEST){a=0;d=lo;}if(face==Facing::EAST){a=hi;d=1;}
        add(a,b,c,d,e,f);
    }
}
bool ChorusPlantTile::canSurvive(Level *level,int x,int y,int z) {
    int below=level->getTile(x,y-1,z);
    bool openAbove=level->isEmptyTile(x,y+1,z),openBelow=below==0;
    for(int face=2;face<6;++face) {
        int nx=x+Facing::STEP_X[face],nz=z+Facing::STEP_Z[face];
        if(level->getTile(nx,y,nz)==id) {
            if(!openAbove&&!openBelow) return false;
            int support=level->getTile(nx,y-1,nz);
            if(support==id||support==Tile::endStone_Id) return true;
        }
    }
    return below==id||below==Tile::endStone_Id;
}
bool ChorusPlantTile::mayPlace(Level *level,int x,int y,int z) { return Tile::mayPlace(level,x,y,z)&&canSurvive(level,x,y,z); }
void ChorusPlantTile::neighborChanged(Level *level,int x,int y,int z,int) {
    if(!level->isClientSide&&!canSurvive(level,x,y,z)) level->addToTickNextTick(x,y,z,id,1);
}
void ChorusPlantTile::tick(Level *level,int x,int y,int z,Random*) {
    if(!level->isClientSide&&!canSurvive(level,x,y,z)) level->destroyTile(x,y,z,true);
}
int ChorusPlantTile::getResource(int,Random*,int) { return Item::chorusFruit_Id; }
int ChorusPlantTile::getResourceCount(Random *random) { return random->nextInt(2); }

ChorusFlowerTile::ChorusFlowerTile(int id):EndPlantTile(id,Material::plant),plantIcon(nullptr),deadIcon(nullptr) { setTicking(true); }
int ChorusFlowerTile::getParts(LevelSource*,int,int,int,int,EndPlantPart *parts) {
    for(int i=0;i<6;++i) parts[i]=CHORUS_FLOWER_PARTS[i];return 6;
}
void ChorusFlowerTile::registerIcons(IconRegister *r) {
    icon=r->registerIcon(L"chorus_flower");deadIcon=r->registerIcon(L"chorus_flower_dead");plantIcon=r->registerIcon(L"chorus_plant");
}
Icon *ChorusFlowerTile::getPartTexture(int texture,int data) { return texture==0?plantIcon:(data==5?deadIcon:icon); }
Icon *ChorusFlowerTile::getTexture(int,int data) { return data==5?deadIcon:icon; }
bool ChorusFlowerTile::canSurvive(Level *level,int x,int y,int z) {
    int below=level->getTile(x,y-1,z);
    if(below==Tile::chorusPlant_Id||below==Tile::endStone_Id) return true;
    if(below!=0) return false;
    int count=0;
    for(int face=2;face<6;++face) {
        int neighbor=level->getTile(x+Facing::STEP_X[face],y,z+Facing::STEP_Z[face]);
        if(neighbor==Tile::chorusPlant_Id) ++count;else if(neighbor!=0) return false;
    }
    return count==1;
}
bool ChorusFlowerTile::mayPlace(Level *level,int x,int y,int z) { return Tile::mayPlace(level,x,y,z)&&canSurvive(level,x,y,z); }
void ChorusFlowerTile::neighborChanged(Level *level,int x,int y,int z,int) {
    if(!level->isClientSide&&!canSurvive(level,x,y,z)) level->addToTickNextTick(x,y,z,id,1);
}
bool ChorusFlowerTile::clearNeighbors(Level *level,int x,int y,int z,int exceptFace) {
    for(int face=2;face<6;++face) if(face!=exceptFace&&!level->isEmptyTile(x+Facing::STEP_X[face],y,z+Facing::STEP_Z[face])) return false;
    return true;
}
void ChorusFlowerTile::tick(Level *level,int x,int y,int z,Random *random) {
    if(level->isClientSide) return;
    if(!canSurvive(level,x,y,z)) {level->destroyTile(x,y,z,true);return;}
    int age=level->getData(x,y,z);
    if(age>=5||y+1>=Level::maxBuildHeight||!level->isEmptyTile(x,y+1,z)) return;
    int below=level->getTile(x,y-1,z),height=1;bool rooted=false,grow=below==0||below==Tile::endStone_Id;
    if(below==Tile::chorusPlant_Id) {
        while(height<5&&level->getTile(x,y-height-1,z)==Tile::chorusPlant_Id) ++height;
        rooted=level->getTile(x,y-height-1,z)==Tile::endStone_Id;
        grow=height<2||height<=random->nextInt(rooted?5:4);
    }
    if(grow&&y+2<Level::maxBuildHeight&&clearNeighbors(level,x,y+1,z)&&level->isEmptyTile(x,y+2,z)) {
        level->setTileAndData(x,y,z,Tile::chorusPlant_Id,0,UPDATE_ALL);
        level->setTileAndData(x,y+1,z,id,age,UPDATE_ALL);return;
    }
    bool branched=false;
    if(age<4) for(int i=0,attempts=random->nextInt(4)+(rooted?1:0);i<attempts;++i) {
        int face=2+random->nextInt(4),nx=x+Facing::STEP_X[face],nz=z+Facing::STEP_Z[face];
        if(level->isEmptyTile(nx,y,nz)&&level->isEmptyTile(nx,y-1,nz)&&clearNeighbors(level,nx,y,nz,Facing::getOpposite(face))) {
            level->setTileAndData(nx,y,nz,id,age+1,UPDATE_ALL);branched=true;
        }
    }
    if(branched) level->setTileAndData(x,y,z,Tile::chorusPlant_Id,0,UPDATE_ALL);
    else level->setData(x,y,z,5,UPDATE_CLIENTS);
}
int ChorusFlowerTile::getResource(int,Random*,int) { return 0; }
int ChorusFlowerTile::getSpawnResourcesAuxValue(int) { return 0; }
void ChorusFlowerTile::playerDestroy(Level *level,shared_ptr<Player> player,int x,int y,int z,int data) {
    Tile::playerDestroy(level,player,x,y,z,data);
    if(!level->isClientSide) popResource(level,x,y,z,std::make_shared<ItemInstance>(id,1,0));
}
