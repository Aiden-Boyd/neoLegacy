#include "stdafx.h"
#include "EndCrystalItem.h"
#include "EnderCrystal.h"
#include "EndFight.h"
#include "ItemInstance.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.phys.h"
EndCrystalItem::EndCrystalItem(int id) : Item(id) {}
bool EndCrystalItem::isFoil(shared_ptr<ItemInstance>) {return true;}
bool EndCrystalItem::useOn(shared_ptr<ItemInstance> instance,shared_ptr<Player> player,Level *level,int x,int y,int z,int face,float,float,float,bool preview)
{
    if(!instance || instance->count<=0 || !player || y<0 || y+2>=Level::genDepth)return false;
    int base=level->getTile(x,y,z);
    if(base!=Tile::obsidian_Id && base!=Tile::unbreakable_Id)return false;
    if(!player->mayUseItemAt(x,y+1,z,face,instance) || !player->mayUseItemAt(x,y+2,z,face,instance))return false;
    for(int dy=1;dy<=2;++dy) {
        if(!level->hasChunkAt(x,y+dy,z))return false;
        if(!level->isEmptyTile(x,y+dy,z) && !level->getMaterial(x,y+dy,z)->isReplaceable())return false;
    }
    auto entities=level->getEntities(nullptr,AABB::newTemp(x,y+1,z,x+1,y+3,z+1));
    bool blocked=false;for(auto &entity:*entities)if(entity && !entity->removed){blocked=true;break;}delete entities;
    if(blocked)return false;
    if(preview || level->isClientSide)return true;
    auto crystal=std::make_shared<EnderCrystal>(level,x+.5,y+1,z+.5);crystal->setShowBottom(false);
    if(!level->addEntity(crystal))return false;
    if(!player->abilities.instabuild)--instance->count;
    EndFight::tryRespawn(level);
    return true;
}
