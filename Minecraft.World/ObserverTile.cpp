#include "stdafx.h"
#include "ObserverTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.entity.h"
#include "Facing.h"
namespace {int facing(int data){int f=data&7;return f<6?f:2;}}
ObserverTile::ObserverTile(int id):Tile(id,Material::stone){sendTileData();}
void ObserverTile::registerIcons(IconRegister*r)
{
    front=r->registerIcon(L"observer_front");back=r->registerIcon(L"observer_back");backOn=r->registerIcon(L"observer_back_on");side=r->registerIcon(L"observer_side");
    for(int i=0;i<4;++i)top[i]=r->registerIcon(L"observer_top_r"+std::to_wstring(i));icon=front;
}
Icon *ObserverTile::getTexture(int face,int data)
{
    int f=facing(data);if(face==f)return front;if(face==(f^1))return(data&8)?backOn:back;
    if(f<2){if(face==2||face==3)return top[f==1?0:2];return side;}
    if(face<2){const int q[6]={0,0,0,2,3,1};return top[(face==1?2+q[f]:4-q[f])&3];}
    return side;
}
int ObserverTile::getSignal(LevelSource*l,int x,int y,int z,int direction)
{int data=l->getData(x,y,z);return (data&8)&&facing(data)==direction?15:0;}
void ObserverTile::notifyOutput(Level*l,int x,int y,int z,int data)
{
    int f=facing(data),back=f^1;int xx=x+Facing::STEP_X[back],yy=y+Facing::STEP_Y[back],zz=z+Facing::STEP_Z[back];
    if(!l->hasChunkAt(xx,yy,zz))return;
    l->neighborChanged(xx,yy,zz,id);l->updateNeighborsAtExceptFromFacing(xx,yy,zz,id,f);
}
void ObserverTile::startPulse(Level*l,int x,int y,int z)
{if(!l->isClientSide && !(l->getData(x,y,z)&8) && !l->isTileToBeTickedAt(x,y,z,id))l->addToTickNextTick(x,y,z,id,2);}
void ObserverTile::onPlace(Level*l,int x,int y,int z)
{
    if(l->isClientSide)return;
    int data=l->getData(x,y,z);
    if(data&8){l->setData(x,y,z,data&7,Tile::UPDATE_CLIENTS);notifyOutput(l,x,y,z,data);}
    startPulse(l,x,y,z);
}
void ObserverTile::onRemove(Level*l,int x,int y,int z,int,int data){if(!l->isClientSide&&(data&8))notifyOutput(l,x,y,z,data);}
void ObserverTile::tick(Level*l,int x,int y,int z,Random*)
{
    if(l->isClientSide || l->getTile(x,y,z)!=id)return;
    int data=l->getData(x,y,z);bool powered=(data&8)!=0;
    l->setData(x,y,z,(data&7)|(powered?0:8),Tile::UPDATE_CLIENTS);
    if(!powered)l->addToTickNextTick(x,y,z,id,2);
    notifyOutput(l,x,y,z,data);
}
void ObserverTile::stateChanged(Level*l,int x,int y,int z,int flags)
{
    if(l->isClientSide || (flags&Tile::UPDATE_SUPPRESS_OBSERVERS) || (flags&Tile::UPDATE_INVISIBLE_NO_LIGHT)==Tile::UPDATE_INVISIBLE_NO_LIGHT)return;
    for(int face=0;face<6;++face) {
        int xx=x+Facing::STEP_X[face],yy=y+Facing::STEP_Y[face],zz=z+Facing::STEP_Z[face];
        if(yy<0 || yy>=Level::maxBuildHeight || !l->hasChunkAt(xx,yy,zz) || l->getTile(xx,yy,zz)!=Tile::observer_Id)continue;
        if(facing(l->getData(xx,yy,zz))==(face^1))static_cast<ObserverTile*>(Tile::tiles[Tile::observer_Id])->startPulse(l,xx,yy,zz);
    }
}
void ObserverTile::setPlacedBy(Level*l,int x,int y,int z,shared_ptr<LivingEntity>by,shared_ptr<ItemInstance>)
{
    if(!by)return;
    int f=2;
    if(std::abs(by->x-x)<2&&std::abs(by->z-z)<2) {
        double eye=by->y+1.82-by->heightOffset;
        if(eye-y>2)f=1;else if(y-eye>0)f=0;else {const int dirs[4]={2,5,3,4};f=dirs[Mth::floor(by->yRot*4/360+.5)&3];}
    } else {const int dirs[4]={2,5,3,4};f=dirs[Mth::floor(by->yRot*4/360+.5)&3];}
    l->setData(x,y,z,f^1,Tile::UPDATE_ALL);
}
