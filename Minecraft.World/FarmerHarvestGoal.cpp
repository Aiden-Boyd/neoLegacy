#include "stdafx.h"
#include "FarmerHarvestGoal.h"
#include "FarmerInventory.h"
#include "Villager.h"
#include "CropTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.phys.h"
#include "GameRules.h"
namespace {
int cropForSeed(int seed) {switch(seed){case 295:return Tile::wheat_Id;case 391:return Tile::carrots_Id;case 392:return Tile::potatoes_Id;case 435:return Tile::beetroots_Id;default:return 0;}}
}
FarmerHarvestGoal::FarmerHarvestGoal(Villager *v) : farmer(v),delay(0),ticks(0),active(false),harvesting(false)
{setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);}
bool FarmerHarvestGoal::allowed()
{
    return !farmer->level->isClientSide && farmer->isAlive() && !farmer->isBaby() && !farmer->isTrading() && farmer->getProfession()==Villager::PROFESSION_FARMER && farmer->level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING);
}
bool FarmerHarvestGoal::valid(int x,int y,int z,bool harvest)
{
    Level *l=farmer->level;
    if(y<=0 || y>=255 || !l->hasChunkAt(x,y,z) || l->getTile(x,y-1,z)!=Tile::farmland_Id)return false;
    int id=l->getTile(x,y,z);
    if(harvest) {
        if(!farmer->getFarmInventory()->wantsFood())return false;
        if(id!=Tile::wheat_Id && id!=Tile::carrots_Id && id!=Tile::potatoes_Id && id!=Tile::beetroots_Id)return false;
        return l->getData(x,y,z)>=static_cast<CropTile*>(Tile::tiles[id])->getMaxAge();
    }
    return id==0 && farmer->getFarmInventory()->plantingItem()!=0;
}
bool FarmerHarvestGoal::canUse()
{
    if(!allowed())return false;
    if(delay>0){--delay;return false;}
    delay=200+farmer->getRandom()->nextInt(200);
    int cx=Mth::floor(farmer->x),cy=Mth::floor(farmer->y),cz=Mth::floor(farmer->z);
    // Expanding rings prefer reachable nearby fields; never load/generate chunks.
    for(int radius=0;radius<=16;++radius)for(int dx=-radius;dx<=radius;++dx)for(int dz=-radius;dz<=radius;++dz) {
        if(abs(dx)!=radius && abs(dz)!=radius)continue;
        for(int dy=-1;dy<=1;++dy)for(int action=0;action<2;++action)if(valid(cx+dx,cy+dy,cz+dz,action==0)) {
            targetX=cx+dx;targetY=cy+dy;targetZ=cz+dz;harvesting=action==0;return true;
        }
    }
    return false;
}
bool FarmerHarvestGoal::canContinueToUse() {return active && ticks<1200 && allowed() && valid(targetX,targetY,targetZ,harvesting);}
void FarmerHarvestGoal::start() {ticks=0;active=farmer->getNavigation()->moveTo(targetX+.5,targetY,targetZ+.5,.6);}
void FarmerHarvestGoal::stop() {farmer->getNavigation()->stop();active=false;delay=10;}
void FarmerHarvestGoal::tick()
{
    if(!canContinueToUse()){active=false;return;}
    ++ticks;
    farmer->getLookControl()->setLookAt(targetX+.5,targetY+.5,targetZ+.5,10,farmer->getMaxHeadXRot());
    double dx=farmer->x-(targetX+.5),dz=farmer->z-(targetZ+.5),dy=farmer->y-targetY;
    if(dx*dx+dz*dz<1.0 && dy>-.6 && dy<1.5) {
        Level *l=farmer->level;
        if(harvesting)l->destroyTile(targetX,targetY,targetZ,true);
        else {
            int seed=farmer->getFarmInventory()->plantingItem(),crop=cropForSeed(seed);
            if(crop && l->setTileAndData(targetX,targetY,targetZ,crop,0,Tile::UPDATE_ALL))farmer->getFarmInventory()->consume(seed);
        }
        active=false;delay=10;
    } else if(ticks%40==0)active=farmer->getNavigation()->moveTo(targetX+.5,targetY,targetZ+.5,.6);
}
