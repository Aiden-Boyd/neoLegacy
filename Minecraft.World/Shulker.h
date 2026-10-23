#pragma once
#include "Monster.h"
#include "SoundTypes.h"
#include "SynchedEntityData.h"
#include "Throwable.h"

class Shulker : public Monster
{
    int shotDelay=20,idlePeek=0;
    float peek=0,oldPeek=0;
    bool supported(int,int,int,int);
    void updateShellBox();
    void setPeek(int);
protected:
    void defineSynchedData() override;
    void registerAttributes() override;
    void serverAiStep() override {}
    bool useNewAi() override {return false;}
    bool removeWhenFarAway() override {return false;}
    int getAmbientSound() override {return entityData->getByte(17)>0?eSoundType_SHULKER_AMBIENT:-1;}
    int getHurtSound() override {return entityData->getByte(17)==0?eSoundType_SHULKER_HURT_CLOSED:eSoundType_SHULKER_HURT;}
    int getDeathSound() override {return eSoundType_SHULKER_DEATH;}
    void dropDeathLoot(bool,int) override;
public:
    explicit Shulker(Level*);
    eINSTANCEOF GetType() override {return eTYPE_SHULKER;}
    static Entity *create(Level *level) {return new Shulker(level);}
    int getAttachFace();
    float getPeek(float);
    bool teleportSomewhere();
    void tick() override;
    void aiStep() override;
    void travel(float,float) override;
    bool isPushable() override {return false;}
    AABB *getCollideBox() override {return isAlive()?bb:nullptr;}
    int getArmorValue() override;
    bool hurt(DamageSource*,float) override;
    void addAdditonalSaveData(CompoundTag*) override;
    void readAdditionalSaveData(CompoundTag*) override;
};

class ShulkerBullet : public Throwable
{
    weak_ptr<LivingEntity> target;
    wstring targetUuid,ownerUuid;
    int age=0,turnDelay=0,direction=1;
    double steerX=0,steerY=.15,steerZ=0;
    void chooseDirection(shared_ptr<LivingEntity>);
protected:
    float getGravity() override {return 0;}
    void onHit(HitResult*) override;
public:
    explicit ShulkerBullet(Level*);
    ShulkerBullet(Level*,shared_ptr<LivingEntity>,shared_ptr<LivingEntity>,int);
    static Entity *create(Level *level) {return new ShulkerBullet(level);}
    eINSTANCEOF GetType() override {return eTYPE_SHULKER_BULLET;}
    void tick() override;
    bool isPickable() override {return true;}
    bool hurt(DamageSource*,float) override;
    void addAdditonalSaveData(CompoundTag*) override;
    void readAdditionalSaveData(CompoundTag*) override;
};
