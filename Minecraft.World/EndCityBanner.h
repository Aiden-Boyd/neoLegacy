#pragma once
#include "BaseEntityTile.h"
#include "TileEntity.h"

// The naturally generated city banner: green, with black upper/lower triangles.
class EndCityBannerEntity : public TileEntity
{
public:
    eINSTANCEOF GetType() override {return eTYPE_ENDCITYBANNERENTITY;}
    static TileEntity *create() {return new EndCityBannerEntity();}
    shared_ptr<TileEntity> clone() override {auto result=std::make_shared<EndCityBannerEntity>();TileEntity::clone(result);return result;}
};
class EndCityBanner : public BaseEntityTile
{
public:
    explicit EndCityBanner(int);
    shared_ptr<TileEntity> newTileEntity(Level*) override {return std::make_shared<EndCityBannerEntity>();}
    bool isSolidRender(bool=false) override {return false;}
    bool isCubeShaped() override {return false;}
    int getRenderShape() override {return SHAPE_INVISIBLE;}
    AABB *getAABB(Level*,int,int,int) override {return nullptr;}
    int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int) override;
    void neighborChanged(Level*,int,int,int,int) override;
};
