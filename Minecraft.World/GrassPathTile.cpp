#include "stdafx.h"
#include "GrassPathTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.phys.h"
#include "IconRegister.h"
GrassPathTile::GrassPathTile(int id) : Tile(id,Material::dirt),side(nullptr)
{
    setLightBlock(255);
    updateDefaultShape();
}
void GrassPathTile::updateDefaultShape() {setShape(0,0,0,1,15.0f/16.0f,1);}
AABB *GrassPathTile::getAABB(Level *level,int x,int y,int z) {return AABB::newTemp(x,y,z,x+1,y+15.0/16.0,z+1);}
bool GrassPathTile::isSolidRender(bool) {return false;}
bool GrassPathTile::isCubeShaped() {return false;}
bool GrassPathTile::shouldRenderFace(LevelSource *level,int x,int y,int z,int face)
{
    if(face==1) return true;
    int adjacent=level->getTile(x,y,z);
    if(face>=2 && (adjacent==id || adjacent==Tile::farmland_Id)) return false;
    return Tile::shouldRenderFace(level,x,y,z,face);
}
Icon *GrassPathTile::getTexture(int face,int data) {return face==1?icon:face==0?Tile::dirt->getTexture(face,0):side;}
void GrassPathTile::registerIcons(IconRegister *icons) {icon=icons->registerIcon(L"grass_path_top");side=icons->registerIcon(L"grass_path_side");}
int GrassPathTile::getResource(int,Random *,int) {return Tile::dirt_Id;}
bool GrassPathTile::isSilkTouchable() {return false;}
void GrassPathTile::onPlace(Level *level,int x,int y,int z) {neighborChanged(level,x,y,z,0);}
void GrassPathTile::neighborChanged(Level *level,int x,int y,int z,int)
{
    if(!level->isClientSide && level->getMaterial(x,y+1,z)->isSolid())
        level->setTileAndData(x,y,z,Tile::dirt_Id,0,Tile::UPDATE_ALL);
}
