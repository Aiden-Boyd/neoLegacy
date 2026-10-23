#include "stdafx.h"
#include "EndCityBanner.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"

EndCityBanner::EndCityBanner(int id):BaseEntityTile(id,Material::wood,false) {setLightBlock(0);}
int EndCityBanner::getPlacedOnFaceDataValue(Level*,int,int,int,int face,float,float,float,int) {return face>=2&&face<=5?face:2;}
void EndCityBanner::neighborChanged(Level *level,int x,int y,int z,int) {
    int face=level->getData(x,y,z),xx=x,zz=z;
    if(face==2) ++zz;else if(face==3) --zz;else if(face==4) ++xx;else if(face==5) --xx;
    if(!level->getMaterial(xx,y,zz)->isSolid()) {
        if(!level->isClientSide) popResource(level,x,y,z,std::make_shared<ItemInstance>(id,1,0));
        level->setTileAndUpdate(x,y,z,0);
    }
}
