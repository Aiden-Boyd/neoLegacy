#include "stdafx.h"
#include "Shulker.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "DamageSource.h"
#include "Difficulty.h"
#include "net.minecraft.world.level.dimension.h"
#include "CompoundTag.h"
#include "SynchedEntityData.h"
#include "MobEffect.h"
#include "MobEffectInstance.h"
#include "Item.h"
#include "ItemInstance.h"
#include <cmath>

namespace {const int dx[6]={0,0,0,0,-1,1},dy[6]={-1,1,0,0,0,0},dz[6]={0,0,-1,1,0,0};}
Shulker::Shulker(Level *level):Monster(level) {
    defineSynchedData();registerAttributes();setHealth(getMaxHealth());setSize(1,1);xpReward=5;
    setPersistenceRequired();
}
void Shulker::defineSynchedData() {Monster::defineSynchedData();entityData->define(16,byte(0));entityData->define(17,byte(0));}
void Shulker::registerAttributes() {
    Monster::registerAttributes();getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16);
}
int Shulker::getAttachFace() {int f=entityData->getByte(16);return f>=0&&f<6?f:0;}
float Shulker::getPeek(float a) {return oldPeek+(peek-oldPeek)*a;}
int Shulker::getArmorValue() {return entityData->getByte(17)==0?20:0;}
bool Shulker::supported(int x,int y,int z,int face) {
    int edge=level->dimension->getXZSize()*8;
    if(x<=-edge||x>=edge-1||z<=-edge||z>=edge-1||y<1||y>=Level::maxBuildHeight-2) return false;
    if(!level->hasChunksAt(x-1,y-1,z-1,x+1,y+2,z+1)) return false;
    int id=level->getTile(x+dx[face],y+dy[face],z+dz[face]);
    if(!id||!Tile::tiles[id]||!Tile::tiles[id]->isCubeShaped()) return false;
    // The closed shell occupies precisely one cell. Opening clearance is tested too.
    AABB *space=AABB::newTemp(x,y,z,x+1,y+1,z+1)->expand(-dx[face],-dy[face],-dz[face]);
    return level->getCubes(shared_from_this(),space)->empty()&&!level->containsAnyLiquid(space);
}
bool Shulker::teleportSomewhere() {
    if(level->isClientSide||!isAlive()||riding) return false;
    int bx=int(std::floor(x)),by=int(std::floor(bb->y0)),bz=int(std::floor(z));
    for(int attempt=0;attempt<5;++attempt) {
        int xx=bx+random->nextInt(17)-8,yy=by+random->nextInt(17)-8,zz=bz+random->nextInt(17)-8;
        for(int face=0;face<6;++face) if(supported(xx,yy,zz,face)) {
            setPos(xx+.5,yy,zz+.5);entityData->set(16,byte(face));entityData->set(17,byte(0));
            setTarget(nullptr);shotDelay=20;fallDistance=0;peek=oldPeek=0;updateShellBox();return true;
        }
    }
    return false;
}
void Shulker::updateShellBox() {
    double xx=std::floor(x),yy=std::floor(y-heightOffset),zz=std::floor(z);
    double lift=.5-.5*std::cos(peek*3.141592653589793);
    int face=getAttachFace();
    bb->set(xx+(dx[face]>0?-lift:0),yy+(dy[face]>0?-lift:0),zz+(dz[face]>0?-lift:0),
        xx+1+(dx[face]<0?lift:0),yy+1+(dy[face]<0?lift:0),zz+1+(dz[face]<0?lift:0));
}
void Shulker::tick() {
    Monster::tick();oldPeek=peek;
    float desired=entityData->getByte(17)/100.f;
    if(peek<desired) peek=(std::min)(desired,peek+.05f);
    if(peek>desired) peek=(std::max)(desired,peek-.05f);
    updateShellBox();
    if(!level->isClientSide&&peek>oldPeek) {
        auto entities=level->getEntities(shared_from_this(),bb);
        for(auto e:*entities) if(e->isPushable()&&!e->instanceof(eTYPE_SHULKER)) {
            int f=getAttachFace();e->move(-dx[f]*.05,-dy[f]*.05,-dz[f]*.05);
        }
    }
}
void Shulker::travel(float,float) {xd=yd=zd=0;fallDistance=0;}
void Shulker::aiStep() {
    Monster::aiStep();if(level->isClientSide||removed) return;
    int xx=int(std::floor(x)),yy=int(std::floor(y-heightOffset)),zz=int(std::floor(z));
    // A shut shell needs no opening clearance, but its support must remain solid.
    int face=getAttachFace(),id=level->getTile(xx+dx[face],yy+dy[face],zz+dz[face]);
    if(!id||!Tile::tiles[id]||!Tile::tiles[id]->isCubeShaped()) {
        bool attached=false;
        for(int f=0;f<6;++f) if(supported(xx,yy,zz,f)) {entityData->set(16,byte(f));attached=true;break;}
        if(!attached) {teleportSomewhere();return;}
    }
    auto target=getTarget();
    if(target&&(!target->isAlive()||target->level!=level||distanceToSqr(target)>400)) {setTarget(nullptr);target=nullptr;}
    if(!target&&level->difficulty!=Difficulty::PEACEFUL) {
        target=level->getNearestAttackablePlayer(shared_from_this(),16);setTarget(target);
    }
    if(target) {
        entityData->set(17,byte(100));lookAt(target,180,180);
        if(--shotDelay<=0&&canSee(target)) {
            auto bullet=std::make_shared<ShulkerBullet>(level,dynamic_pointer_cast<LivingEntity>(shared_from_this()),target,getAttachFace());
            level->addEntity(bullet);shotDelay=20+random->nextInt(10)*10;
        }
    } else {
        shotDelay=20;
        if(idlePeek>0) {--idlePeek;entityData->set(17,byte(30));}
        else {entityData->set(17,byte(0));if(random->nextInt(40)==0&&supported(xx,yy,zz,getAttachFace())) idlePeek=20+random->nextInt(3)*20;}
    }
}
bool Shulker::hurt(DamageSource *source,float amount) {
    auto direct=source->getDirectEntity();if(entityData->getByte(17)==0&&direct&&direct->instanceof(eTYPE_ARROW)) return false;
    bool hit=Monster::hurt(source,amount);
    if(hit&&!level->isClientSide&&isAlive()&&getHealth()<15&&random->nextInt(4)==0) teleportSomewhere();return hit;
}
void Shulker::dropDeathLoot(bool,int bonus) {if(random->nextFloat()<.5f+bonus*.0625f) spawnAtLocation(Item::shulkerShell_Id,1);}
void Shulker::addAdditonalSaveData(CompoundTag *tag) {Monster::addAdditonalSaveData(tag);tag->putByte(L"AttachFace",byte(getAttachFace()));tag->putByte(L"Peek",entityData->getByte(17));}
void Shulker::readAdditionalSaveData(CompoundTag *tag) {Monster::readAdditionalSaveData(tag);int f=tag->getByte(L"AttachFace");entityData->set(16,byte(f>=0&&f<6?f:0));int p=tag->getByte(L"Peek");entityData->set(17,byte(p>=0&&p<=100?p:0));}

ShulkerBullet::ShulkerBullet(Level *level):Throwable(level) {setSize(.3125f,.3125f);}
ShulkerBullet::ShulkerBullet(Level *level,shared_ptr<LivingEntity> owner,shared_ptr<LivingEntity> victim,int face):Throwable(level,owner),target(victim) {
    setSize(.3125f,.3125f);setPos(owner->x-dx[face]*.6,owner->y+.5-dy[face]*.6,owner->z-dz[face]*.6);
    ownerUuid=owner->getUUID();targetUuid=victim->getUUID();xd=yd=zd=0;direction=face^1;chooseDirection(victim);
}
void ShulkerBullet::chooseDirection(shared_ptr<LivingEntity> victim) {
    double delta[3]={victim->x-x,victim->y+victim->bbHeight*.5-y,victim->z-z};
    vector<int> choices;
    for(int f=0;f<6;++f) {
        if((f/2)==(direction/2)) continue;
        if((dx[f]*delta[0]+dy[f]*delta[1]+dz[f]*delta[2])<=0) continue;
        if(level->isEmptyTile(int(std::floor(x))+dx[f],int(std::floor(y))+dy[f],int(std::floor(z))+dz[f])) choices.push_back(f);
    }
    if(choices.empty()) for(int f=0;f<6;++f) if(level->isEmptyTile(int(std::floor(x))+dx[f],int(std::floor(y))+dy[f],int(std::floor(z))+dz[f])) choices.push_back(f);
    if(!choices.empty()) direction=choices[random->nextInt(int(choices.size()))];
    double length=std::sqrt(delta[0]*delta[0]+delta[1]*delta[1]+delta[2]*delta[2]);
    if(length<2&&length>.001) {steerX=delta[0]/length*.15;steerY=delta[1]/length*.15;steerZ=delta[2]/length*.15;}
    else {steerX=dx[direction]*.15;steerY=dy[direction]*.15;steerZ=dz[direction]*.15;}
    turnDelay=10+random->nextInt(5)*10;
}
void ShulkerBullet::tick() {
    if(!level->isClientSide) {
        if(level->difficulty==Difficulty::PEACEFUL||++age>600) {remove();return;}
        auto victim=target.lock();
        if((!victim&&!targetUuid.empty()||!owner&&!ownerUuid.empty())&&age%20==1) {
            auto candidates=level->getEntities(shared_from_this(),bb->grow(64,64,64));
            for(auto e:*candidates) {if(e->getUUID()==targetUuid) {victim=dynamic_pointer_cast<LivingEntity>(e);target=victim;} if(e->getUUID()==ownerUuid) owner=dynamic_pointer_cast<LivingEntity>(e);}
        }
        if(victim&&victim->isAlive()&&victim->level==level) {
            bool crossed=dx[direction]*(victim->x-x)+dy[direction]*(victim->y+victim->bbHeight*.5-y)+dz[direction]*(victim->z-z)<=0;
            if(--turnDelay<=0||crossed||!level->isEmptyTile(int(std::floor(x))+dx[direction],int(std::floor(y))+dy[direction],int(std::floor(z))+dz[direction])) chooseDirection(victim);
            steerX=(std::max)(-.6,(std::min)(.6,steerX*1.025));steerY=(std::max)(-.6,(std::min)(.6,steerY*1.025));steerZ=(std::max)(-.6,(std::min)(.6,steerZ*1.025));
            xd+=(steerX-xd)*.2;yd+=(steerY-yd)*.2;zd+=(steerZ-zd)*.2;
        } else yd-=.04;
    } else level->addParticle(eParticleType_endRod,x,y,z,0,0,0);
    Throwable::tick();
}
void ShulkerBullet::onHit(HitResult *hit) {
    if(level->isClientSide) return;
    if(hit->type==HitResult::ENTITY&&hit->entity) {
        auto victim=dynamic_pointer_cast<LivingEntity>(hit->entity);
        DamageSource *damage=DamageSource::thrown(shared_from_this(),getOwner());
        if(hit->entity->hurt(damage,4)&&victim) victim->addEffect(new MobEffectInstance(MobEffect::levitation->id,200,0));
        delete damage;
    }
    remove();
}
bool ShulkerBullet::hurt(DamageSource*,float) {if(level->isClientSide) return false;remove();return true;}
void ShulkerBullet::addAdditonalSaveData(CompoundTag *tag) {
    Throwable::addAdditonalSaveData(tag);tag->putString(L"OwnerUUID",ownerUuid);tag->putString(L"Target",targetUuid);tag->putInt(L"Age",age);tag->putInt(L"Direction",direction);tag->putInt(L"TurnDelay",turnDelay);
    tag->putDouble(L"SteerX",steerX);tag->putDouble(L"SteerY",steerY);tag->putDouble(L"SteerZ",steerZ);
}
void ShulkerBullet::readAdditionalSaveData(CompoundTag *tag) {
    Throwable::readAdditionalSaveData(tag);ownerUuid=tag->getString(L"OwnerUUID");targetUuid=tag->getString(L"Target");age=(std::max)(0,(std::min)(600,tag->getInt(L"Age")));
    direction=tag->getInt(L"Direction");if(direction<0||direction>5) direction=1;turnDelay=(std::max)(0,(std::min)(50,tag->getInt(L"TurnDelay")));
    auto bounded=[](double v){return std::isfinite(v)?(std::max)(-.6,(std::min)(.6,v)):0.;};
    steerX=bounded(tag->getDouble(L"SteerX"));steerY=bounded(tag->getDouble(L"SteerY"));steerZ=bounded(tag->getDouble(L"SteerZ"));
}
