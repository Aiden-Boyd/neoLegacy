#include "stdafx.h"
#include "ChorusFruitItem.h"
#include "ItemInstance.h"
#include "Random.h"
#include "SoundTypes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "Material.h"
#include <algorithm>
#include <cmath>

ChorusFruitItem::ChorusFruitItem(int id):FoodItem(id,4,.3f,false) { setCanAlwaysEat(); }
shared_ptr<ItemInstance> ChorusFruitItem::use(shared_ptr<ItemInstance> item,Level*,shared_ptr<Player> player) {
    // Chorus is usable at full hunger, including in Creative.
    if(player->chorusFruitCooldown==0) player->startUsingItem(item,getUseDuration(item));
    return item;
}
shared_ptr<ItemInstance> ChorusFruitItem::useTimeDepleted(shared_ptr<ItemInstance> item,Level *level,shared_ptr<Player> player) {
    if(player->chorusFruitCooldown>0) return item;
    int count=item->count;
    FoodItem::useTimeDepleted(item,level,player);
    if(player->abilities.instabuild) item->count=count;
    player->chorusFruitCooldown=20;
    if(!level->isClientSide) teleport(level,player);
    return item;
}
bool ChorusFruitItem::teleport(Level *level,shared_ptr<Player> player) {
    if(level->isClientSide) return false;
    const double oldX=player->x,oldY=player->y,oldZ=player->z;
    for(int attempt=0;attempt<16;++attempt) {
        double x=oldX+(level->random->nextDouble()-.5)*16;
        double y=std::max(0.,std::min(double(Level::maxBuildHeight-1),oldY+level->random->nextInt(16)-8));
        double z=oldZ+(level->random->nextDouble()-.5)*16;
        int bx=int(std::floor(x)),by=int(std::floor(y)),bz=int(std::floor(z));
        if(!level->hasChunkAt(bx,by,bz)) continue;
        while(by>0&&!level->getMaterial(bx,by-1,bz)->blocksMotion()) { --by;--y; }
        if(by<=0) continue;
        double dx=x-oldX,dy=y-oldY,dz=z-oldZ;
        AABB *target=AABB::newTemp(player->bb->x0+dx,player->bb->y0+dy,player->bb->z0+dz,
                                  player->bb->x1+dx,player->bb->y1+dy,player->bb->z1+dz);
        if(target->y0<0||target->y1>Level::maxBuildHeight) continue;
        if(!level->hasChunksAt(int(std::floor(target->x0)),int(std::floor(target->y0)),int(std::floor(target->z0)),
                               int(std::floor(target->x1)),int(std::floor(target->y1)),int(std::floor(target->z1)))) continue;
        if(!level->getCubes(player,target)->empty()||level->containsAnyLiquid(target)) continue;
        if(player->riding) player->ride(nullptr);
        player->teleportTo(x,y,z); // ServerPlayer's override synchronizes the connection.
        player->fallDistance=0;
        level->playSound(oldX,oldY,oldZ,eSoundType_MOB_ENDERMEN_PORTAL,1,1);
        level->playEntitySound(player,eSoundType_MOB_ENDERMEN_PORTAL,1,1);
        return true;
    }
    return false;
}
