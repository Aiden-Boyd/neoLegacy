#include "stdafx.h"
#include "EndCityBanner.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.network.packet.h"
#include "ItemInstance.h"
#include "CompoundTag.h"
void EndCityBannerEntity::load(CompoundTag*t){TileEntity::load(t);banner.load(t);creativeBreak=dropHandled=false;}
void EndCityBannerEntity::save(CompoundTag*t){TileEntity::save(t);banner.save(t);}
shared_ptr<Packet>EndCityBannerEntity::getUpdatePacket(){auto t=new CompoundTag();save(t);return std::make_shared<TileEntityDataPacket>(x,y,z,TileEntityDataPacket::TYPE_BANNER,t);}
shared_ptr<TileEntity>EndCityBannerEntity::clone(){auto r=std::make_shared<EndCityBannerEntity>();TileEntity::clone(r);r->banner=banner;return r;}
shared_ptr<ItemInstance>EndCityBannerEntity::asItem(){auto item=std::make_shared<ItemInstance>(425,1,banner.base&15);auto root=new CompoundTag();auto data=new CompoundTag();banner.save(data);root->put(L"BlockEntityTag",data);item->setTag(root);if(!banner.name.empty())item->setHoverName(banner.name);return item;}
EndCityBanner::EndCityBanner(int id):BaseEntityTile(id,Material::wood,false){setLightBlock(0);}
shared_ptr<TileEntity>EndCityBanner::newTileEntity(Level*){auto b=std::make_shared<EndCityBannerEntity>();if(id==176)b->banner.plain();return b;}
bool EndCityBanner::mayPlace(Level*l,int x,int y,int z,int face){
 if(!BaseEntityTile::mayPlace(l,x,y,z))return false;
 if(id==176)return face==1&&l->getMaterial(x,y-1,z)->blocksMotion();
 if(face<2||face>5)return false;
 int xx=x,zz=z;if(face==2)++zz;if(face==3)--zz;if(face==4)++xx;if(face==5)--xx;
 return l->getMaterial(xx,y,zz)->blocksMotion();
}
int EndCityBanner::getPlacedOnFaceDataValue(Level*,int,int,int,int face,float,float,float,int){return id==176?0:face>=2&&face<=5?face:2;}
void EndCityBanner::setPlacedBy(Level*l,int x,int y,int z,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>item){
 auto b=dynamic_pointer_cast<EndCityBannerEntity>(l->getTileEntity(x,y,z));if(!b||!item)return;
 b->banner.plain(item->getAuxValue());
 if(item->hasTag()&&item->getTag()->contains(L"BlockEntityTag",Tag::TAG_Compound))b->banner.load(item->getTag()->getCompound(L"BlockEntityTag"),item->getAuxValue());
 if(item->hasCustomHoverName())b->banner.name=item->getHoverName();b->setChanged();
 l->sendTileUpdated(x,y,z);
}
void EndCityBanner::neighborChanged(Level*l,int x,int y,int z,int){
 int face=l->getData(x,y,z),xx=x,yy=y,zz=z;
 if(id==176)--yy;else if(face==2)++zz;else if(face==3)--zz;else if(face==4)++xx;else if(face==5)--xx;
 if(!l->isClientSide&&!l->getMaterial(xx,yy,zz)->isSolid())l->setTileAndUpdate(x,y,z,0);
}
void EndCityBanner::playerWillDestroy(Level*l,int x,int y,int z,int,shared_ptr<Player>p){auto b=dynamic_pointer_cast<EndCityBannerEntity>(l->getTileEntity(x,y,z));if(b)b->creativeBreak=p->abilities.instabuild;}
void EndCityBanner::onRemove(Level*l,int x,int y,int z,int old,int data){
 auto b=dynamic_pointer_cast<EndCityBannerEntity>(l->getTileEntity(x,y,z));
 if(!l->isClientSide&&b&&!b->dropHandled){b->dropHandled=true;if(!b->creativeBreak)popResource(l,x,y,z,b->asItem());}
 BaseEntityTile::onRemove(l,x,y,z,old,data);
}
