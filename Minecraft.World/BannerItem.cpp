#include "stdafx.h"
#include "BannerItem.h"
#include "EndCityBanner.h"
#include "ItemInstance.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "Facing.h"
#include "Mth.h"
int BannerItem::getColor(shared_ptr<ItemInstance>item,int){return BannerData::color(item?item->getAuxValue():15);}
bool BannerItem::useOn(shared_ptr<ItemInstance>item,shared_ptr<Player>p,Level*l,int x,int y,int z,int face,float,float,float,bool preview){
 if(!item||item->count<=0||!p||face<1||face>5)return false;
 if(!l->getMaterial(x,y,z)->isReplaceable()){x+=Facing::STEP_X[face];y+=Facing::STEP_Y[face];z+=Facing::STEP_Z[face];}
 if(y<1||y>=Level::maxBuildHeight||!l->hasChunkAt(x,y,z)||!p->mayUseItemAt(x,y,z,face,item))return false;
 int id=face==1?176:177;auto tile=static_cast<EndCityBanner*>(Tile::tiles[id]);
 if(!tile||!l->getMaterial(x,y,z)->isReplaceable()||!tile->mayPlace(l,x,y,z,face))return false;
 if(preview||l->isClientSide)return true;
 int data=face==1?(Mth::floor((p->yRot+180)*16/360.f+.5f)&15):face;
 if(!l->setTileAndData(x,y,z,id,data,Tile::UPDATE_ALL))return false;
 tile->setPlacedBy(l,x,y,z,p,item);if(!p->abilities.instabuild)--item->count;return true;
}
