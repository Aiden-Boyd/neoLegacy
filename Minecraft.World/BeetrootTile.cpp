#include "stdafx.h"
#include "BeetrootTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "IconRegister.h"

BeetrootTile::BeetrootTile(int id) : CropTile(id)
{
    for(int i=0;i<4;++i) stages[i]=nullptr;
    updateDefaultShape();
}
int BeetrootTile::getMaxAge() { return 3; }
int BeetrootTile::getBaseSeedId() { return Item::beetrootSeeds_Id; }
int BeetrootTile::getBasePlantId() { return Item::beetroot_Id; }
int BeetrootTile::getBonemealAgeIncrease(Level *level) { return CropTile::getBonemealAgeIncrease(level)/3; }
void BeetrootTile::tick(Level *level,int x,int y,int z,Random *random)
{
    if(level->isClientSide || level->getTile(x,y,z)!=id) return;
    if(random->nextInt(3)==0) Bush::tick(level,x,y,z,random);
    else CropTile::tick(level,x,y,z,random);
}
Icon *BeetrootTile::getTexture(int face,int data) { return stages[max(0,min(data,3))]; }
void BeetrootTile::registerIcons(IconRegister *icons)
{
    const wchar_t *names[]={L"beetroots_stage_0",L"beetroots_stage_1",L"beetroots_stage_2",L"beetroots_stage_3"};
    for(int i=0;i<4;++i) stages[i]=icons->registerIcon(names[i]);
}
void BeetrootTile::updateDefaultShape() { setShape(0,0,0,1,0.125f,1); }
void BeetrootTile::updateShape(LevelSource *level,int x,int y,int z,int forceData,shared_ptr<TileEntity> entity)
{
    int stage=max(0,min(forceData>=0?forceData:level->getData(x,y,z),3));setShape(0,0,0,1,(stage+1)*0.125f,1);
}
void BeetrootTile::spawnResources(Level *level,int x,int y,int z,int data,float odds,int fortune)
{
    Bush::spawnResources(level,x,y,z,data,odds,0);
    if(level->isClientSide || data<getMaxAge()) return;
    for(int i=0;i<3+fortune;++i)
        if(level->random->nextInt(2*getMaxAge())<=data)
            popResource(level,x,y,z,std::make_shared<ItemInstance>(getBaseSeedId(),1,0));
}
