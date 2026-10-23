#include "stdafx.h"
#include "EndCityFeature.h"
#include "EndCityLayout.h"
#include "TheEndLevelRandomLevelSource.h"
#include "EndIslandTerrain.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.tile.h"
#include "LevelData.h"
#include "Shulker.h"
#include "ChestTileEntity.h"
#include "BrewingStandTileEntity.h"
#include "SkullTileEntity.h"
#include "ItemFrame.h"
#include "ItemInstance.h"
#include "Item.h"
#include "PotionItem.h"
#include "EnchantmentHelper.h"
#include "Direction.h"
#include <set>
#include <tuple>

namespace {
int siteHeight(TheEndLevelRandomLevelSource *source,int cx,int cz,int rotation) {
    byteArray blocks(Level::genDepth*256);BiomeArray biomes;
    source->prepareHeights(cx,cz,blocks,biomes);
    int result=Level::genDepth;
    for(int a:{0,5}) for(int b:{0,5}) {
        auto p=EndCityLayout::rotate({a,0,b},rotation);int x=8+p.x,z=8+p.z,top=0;
        for(int y=Level::genDepth-1;y>=0;--y) if(blocks[(x<<Level::genDepthBitsPlusFour)|(z<<Level::genDepthBits)|y]) {top=y+1;break;}
        result=(std::min)(result,top);
    }
    delete[] blocks.data;return result;
}
struct Treasure {int id,weight,low,high;bool enchanted;};
void fillChest(shared_ptr<ChestTileEntity> chest,Random &random) {
    // Reference end_city_treasure weights/counts. Agriculture is a later milestone;
    // unsupported beetroot seeds are omitted rather than exposing an unusable item.
    const Treasure entries[]={
        {264,5,2,7,false},{265,10,4,8,false},{266,15,2,7,false},{388,2,2,6,false},
        {329,3,1,1,false},{417,1,1,1,false},{418,1,1,1,false},{419,1,1,1,false},
        {276,3,1,1,true},{313,3,1,1,true},{311,3,1,1,true},{312,3,1,1,true},{310,3,1,1,true},{278,3,1,1,true},{277,3,1,1,true},
        {267,3,1,1,true},{309,3,1,1,true},{307,3,1,1,true},{308,3,1,1,true},{306,3,1,1,true},{257,3,1,1,true},{256,3,1,1,true}
    };
    int total=0;for(auto e:entries) total+=e.weight;
    for(int roll=0,n=2+random.nextInt(5);roll<n;++roll) {
        int choice=random.nextInt(total);const Treasure *selected=&entries[0];
        for(auto &entry:entries) {choice-=entry.weight;if(choice<0) {selected=&entry;break;}}
        auto item=std::make_shared<ItemInstance>(selected->id,selected->low+random.nextInt(selected->high-selected->low+1),0);
        if(selected->enchanted) item=EnchantmentHelper::enchantItem(&random,item,20+random.nextInt(20),true);
        // Spread rolls across empty slots; never silently overwrite an earlier roll.
        int slot=random.nextInt(chest->getContainerSize());
        for(unsigned i=0;i<chest->getContainerSize()&&chest->getItem(slot);++i) slot=(slot+1)%chest->getContainerSize();
        chest->setItem(slot,item);
    }
    chest->setChanged();
}
void placePart(Level *level,const EndCityLayout::Part &part,int cx,int cz) {
    int index=0;
    for(int n=0;n<part.room->runCount;++n) {
        auto run=part.room->runs[n];
        for(int repeat=0;repeat<run.count;++repeat,++index) {
            if(!part.overwrite&&run.id==0) continue;
            int x=index%part.room->sx,z=(index/part.room->sx)%part.room->sz,y=index/(part.room->sx*part.room->sz);
            auto p=part.world({x,y,z});if(EndCityLayout::floorDiv(p.x,16)!=cx||EndCityLayout::floorDiv(p.z,16)!=cz) continue;
            level->setTileAndData(p.x,p.y,p.z,run.id,EndCityLayout::rotateData(run.id,run.data,part.rotation),Tile::UPDATE_CLIENTS);
        }
    }
}
void markers(Level *level,const EndCityLayout::Part &part,int cx,int cz,std::set<std::tuple<int,int,int,int>> &done) {
    for(int i=0;i<part.room->specialCount;++i) {
        auto special=part.room->specials[i];auto p=part.world({special.x,special.y,special.z});
        if(EndCityLayout::floorDiv(p.x,16)!=cx||EndCityLayout::floorDiv(p.z,16)!=cz) continue;
        if(!done.insert({p.x,p.y,p.z,special.kind}).second) continue;
        if(special.kind==0) {
            if(!level->isEmptyTile(p.x,p.y,p.z)) continue;
            auto shulker=std::make_shared<Shulker>(level);shulker->moveTo(p.x+.5,p.y,p.z+.5,0,0);level->addEntity(shulker);
        } else if(special.kind==1) {
            auto chest=dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(p.x,p.y-1,p.z));
            if(chest) {Random random(level->getSeed()^int64_t(p.x)*341873128712LL^int64_t(p.z)*132897987541LL^p.y);fillChest(chest,random);}
        } else if(special.kind==2) {
            // This engine anchors frames to the supporting block, whereas the
            // template marker denotes the air cell in front of that block.
            auto support=part.world({special.x,special.y,special.z-1});
            auto frame=std::make_shared<ItemFrame>(level,support.x,support.y,support.z,(Direction::SOUTH+part.rotation)&3);
            frame->setItem(std::make_shared<ItemInstance>(Item::elytra,1),false);level->addEntity(frame);
        } else if(special.kind==3) {
            auto skull=dynamic_pointer_cast<SkullTileEntity>(level->getTileEntity(p.x,p.y,p.z));
            if(skull) {skull->setSkullType(5,L"");skull->setRotation(part.rotation*4);skull->setChanged();}
        } else if(special.kind==4) {
            auto stand=dynamic_pointer_cast<BrewingStandTileEntity>(level->getTileEntity(p.x,p.y,p.z));
            if(stand) {stand->setItem(0,std::make_shared<ItemInstance>(Item::potion,1,8229));stand->setItem(2,std::make_shared<ItemInstance>(Item::potion,1,8229));stand->setChanged();}
        }
    }
}
}
void EndCityFeature::postProcess(Level *level,TheEndLevelRandomLevelSource *source,int cx,int cz) {
    if(level->isClientSide||level->dimension->id!=1||level->getLevelData()->hasEndCityChunk(cx,cz)) return;
    int edge=level->dimension->getXZSize()*8;if(edge<1200) return;
    std::vector<EndCityLayout::Plan> plans;
    // Layouts fit inside a 160-block radius. Only inspect regions that can
    // intersect this chunk, without asking the chunk cache for terrain samples.
    for(int rx=EndCityLayout::floorDiv(cx-10,20);rx<=EndCityLayout::floorDiv(cx+10,20);++rx)
    for(int rz=EndCityLayout::floorDiv(cz-10,20);rz<=EndCityLayout::floorDiv(cz+10,20);++rz) {
        int sx,sz;EndCityLayout::site(level->getSeed(),rx,rz,sx,sz);
        if(!EndIslandTerrain::isOuterChunk(sx,sz)||sx*16< -edge+160||sx*16>=edge-160||sz*16< -edge+160||sz*16>=edge-160) continue;
        Random rotationRandom(level->getSeed()^int64_t(sx)*10387313LL^int64_t(sz)*83492791LL);int rotation=rotationRandom.nextInt(4);
        int height=siteHeight(source,sx,sz,rotation);if(height<60) continue;
        EndCityLayout::Plan plan(level->getSeed(),sx,sz,height,rotation);if(!plan.valid(edge,Level::maxBuildHeight)) continue;
        bool intersects=false;
        for(auto &part:plan.parts) {int x0,y0,z0,x1,y1,z1;part.bounds(x0,y0,z0,x1,y1,z1);if(x1>=cx*16&&x0<cx*16+16&&z1>=cz*16&&z0<cz*16+16) {intersects=true;break;}}
        if(intersects) plans.push_back(plan);
    }
    if(plans.empty()) return;
    // Persist the population decision, independently of the disabled generic
    // structure-save framework. Also prevents recursive population from adding
    // the same sentries/loot while native tile placement updates neighbors.
    level->getLevelData()->markEndCityChunk(cx,cz);
    for(auto &plan:plans) for(auto &part:plan.parts) placePart(level,part,cx,cz);
    std::set<std::tuple<int,int,int,int>> done;
    for(auto &plan:plans) for(auto &part:plan.parts) markers(level,part,cx,cz,done);
}
