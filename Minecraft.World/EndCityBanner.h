#pragma once
#include "BaseEntityTile.h"
#include "TileEntity.h"
#include "BannerData.h"
class EndCityBannerEntity:public TileEntity {
public:
 BannerData banner;
 bool creativeBreak=false,dropHandled=false;
 eINSTANCEOF GetType() override{return eTYPE_ENDCITYBANNERENTITY;}
 static TileEntity*create(){return new EndCityBannerEntity();}
 void load(CompoundTag*) override;
 void save(CompoundTag*) override;
 shared_ptr<Packet>getUpdatePacket() override;
 shared_ptr<TileEntity>clone() override;
 shared_ptr<ItemInstance>asItem();
};
class EndCityBanner:public BaseEntityTile {
public:
 explicit EndCityBanner(int);
 shared_ptr<TileEntity>newTileEntity(Level*) override;
 bool isSolidRender(bool=false) override{return false;}
 bool isCubeShaped() override{return false;}
 int getRenderShape() override{return SHAPE_INVISIBLE;}
 int getResourceCount(Random*) override{return 0;}
 int getPistonPushReaction() override{return Material::PUSH_DESTROY;}
 AABB*getAABB(Level*,int,int,int) override{return nullptr;}
 bool mayPlace(Level*,int,int,int,int) override;
 int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int) override;
 void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>) override;
 void neighborChanged(Level*,int,int,int,int) override;
 void playerWillDestroy(Level*,int,int,int,int,shared_ptr<Player>) override;
 void onRemove(Level*,int,int,int,int,int) override;
};
