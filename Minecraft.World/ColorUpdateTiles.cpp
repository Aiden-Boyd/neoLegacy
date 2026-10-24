#include "stdafx.h"
#include "ColorUpdateTiles.h"
#include "FallingTile.h"
#include "IconRegister.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.entity.h"
#include "Facing.h"
namespace {
const wchar_t *colorNames[]={L"white",L"orange",L"magenta",L"light_blue",L"yellow",L"lime",L"pink",L"gray",L"silver",L"cyan",L"purple",L"blue",L"brown",L"green",L"red",L"black"};
}
ConcreteTile::ConcreteTile(int id):Tile(id,Material::stone) {sendTileData();}
void ConcreteTile::registerIcons(IconRegister *r){for(int i=0;i<16;++i)colors[i]=r->registerIcon(std::wstring(L"concrete_")+colorNames[i]);icon=colors[0];}
Icon *ConcreteTile::getTexture(int,int data){return colors[data&15];}
int ConcreteTile::getSpawnResourcesAuxValue(int data){return data&15;}
ConcretePowderTile::ConcretePowderTile(int id):HeavyTile(id) {sendTileData();}
void ConcretePowderTile::registerIcons(IconRegister *r){for(int i=0;i<16;++i)colors[i]=r->registerIcon(std::wstring(L"concrete_powder_")+colorNames[i]);icon=colors[0];}
Icon *ConcretePowderTile::getTexture(int,int data){return colors[data&15];}
int ConcretePowderTile::getSpawnResourcesAuxValue(int data){return data&15;}
bool ConcretePowderTile::harden(Level *l,int x,int y,int z)
{
    if(l->isClientSide || l->getTile(x,y,z)!=id)return false;
    bool water=false;
    for(int face=1;face<6;++face) { // Water below a stationary powder block does not harden it.
        int xx=x+Facing::STEP_X[face],yy=y+Facing::STEP_Y[face],zz=z+Facing::STEP_Z[face];
        if(l->hasChunkAt(xx,yy,zz) && l->getMaterial(xx,yy,zz)==Material::water){water=true;break;}
    }
    return water && l->setTileAndData(x,y,z,Tile::concrete_Id,l->getData(x,y,z)&15,Tile::UPDATE_ALL);
}
void ConcretePowderTile::onPlace(Level*l,int x,int y,int z){if(!l->isClientSide&&!harden(l,x,y,z))HeavyTile::onPlace(l,x,y,z);}
void ConcretePowderTile::neighborChanged(Level*l,int x,int y,int z,int type){if(!l->isClientSide&&!harden(l,x,y,z))HeavyTile::neighborChanged(l,x,y,z,type);}
void ConcretePowderTile::onLand(Level*l,int x,int y,int z,int){harden(l,x,y,z);}
void ConcretePowderTile::tick(Level*l,int x,int y,int z,Random*)
{
    if(l->isClientSide || l->getTile(x,y,z)!=id || harden(l,x,y,z))return;
    if(y<=0 || !HeavyTile::isFree(l,x,y-1,z))return;
    int color=l->getData(x,y,z)&15;
    if(HeavyTile::instaFall || !l->hasChunksAt(x-32,y-32,z-32,x+32,y+32,z+32)) {
        int bottom=y;bool water=false;
        while(bottom>0 && HeavyTile::isFree(l,x,bottom-1,z)) {
            --bottom;
            if(l->getMaterial(x,bottom,z)==Material::water){water=true;break;}
        }
        if(bottom>0 && l->removeTile(x,y,z))l->setTileAndData(x,bottom,z,water?Tile::concrete_Id:id,color,Tile::UPDATE_ALL);
    } else {
        if(!l->newFallingTileAllowed()){l->addToTickNextTick(x,y,z,id,2);return;}
        auto entity=std::make_shared<FallingTile>(l,x+.5,y+.5,z+.5,id,color);
        if(!l->addEntity(entity))l->addToTickNextTick(x,y,z,id,2);
    }
}
bool ConcretePowderTile::hardenFalling(FallingTile *e)
{
    Level *l=e->level;if(l->isClientSide || e->tile!=Tile::concretePowder_Id)return false;
    // Traverse every crossed voxel, including fast falls through a single water layer.
    double dx=e->x-e->xo,dy=e->y-e->yo,dz=e->z-e->zo;
    int x=Mth::floor(e->xo),y=Mth::floor(e->yo),z=Mth::floor(e->zo);
    int endX=Mth::floor(e->x),endY=Mth::floor(e->y),endZ=Mth::floor(e->z);
    int sx=dx>0?1:dx<0?-1:0,sy=dy>0?1:dy<0?-1:0,sz=dz>0?1:dz<0?-1:0;
    const double inf=1e30;
    double tx=sx?((sx>0?x+1:x)-e->xo)/dx:inf,ty=sy?((sy>0?y+1:y)-e->yo)/dy:inf,tz=sz?((sz>0?z+1:z)-e->zo)/dz:inf;
    for(int step=0;step<64;++step) {
        if(y>=0&&y<Level::maxBuildHeight&&l->hasChunkAt(x,y,z)&&l->getMaterial(x,y,z)==Material::water) {
            if(l->setTileAndData(x,y,z,Tile::concrete_Id,e->data&15,Tile::UPDATE_ALL)){e->remove();return true;}
            return false;
        }
        if(x==endX&&y==endY&&z==endZ)break;
        if(tx<=ty&&tx<=tz){x+=sx;tx+=sx?std::abs(1/dx):inf;}
        else if(ty<=tz){y+=sy;ty+=sy?std::abs(1/dy):inf;}
        else {z+=sz;tz+=sz?std::abs(1/dz):inf;}
    }
    return false;
}
GlazedTerracottaTile::GlazedTerracottaTile(int id,int c):Tile(id,Material::stone),color(c){sendTileData(3);}
void GlazedTerracottaTile::registerIcons(IconRegister *r){for(int i=0;i<4;++i)rotations[i]=r->registerIcon(std::wstring(L"glazed_terracotta_")+colorNames[color]+L"_r"+std::to_wstring(i));icon=rotations[0];}
int GlazedTerracottaTile::textureRotation(int face,int direction)
{
    int q=direction&3;
    if(face<0 || face>5)return q;
    if(face==0)return (4-q)&3;
    if(face==1)return q;
    const int base[6]={0,0,1,3,0,2};return (base[face]+4-q)&3;
}
Icon *GlazedTerracottaTile::getTexture(int face,int data){return rotations[textureRotation(face,data)];}
void GlazedTerracottaTile::setPlacedBy(Level*l,int x,int y,int z,shared_ptr<LivingEntity>by,shared_ptr<ItemInstance>)
{if(by)l->setData(x,y,z,(Mth::floor(by->yRot*4.0f/360+.5f)+2)&3,Tile::UPDATE_ALL);}
