#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "FireworksRocketEntity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.phys.h"
#include "DamageSource.h"
#include "HitResult.h"
#include "com.mojang.nbt.h"

FireworksRocketEntity::FireworksRocketEntity(Level *level) : Entity(level)
{
	defineSynchedData();

	life = 0;
	lifetime = 0;
	setSize(0.25f, 0.25f);
}

void FireworksRocketEntity::defineSynchedData()
{
	entityData->defineNULL(DATA_ID_FIREWORKS_ITEM, nullptr);
	entityData->define(DATA_ID_BOOSTED_ENTITY,0);
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distance)
{
	return distance < 64 * 64;
}

FireworksRocketEntity::FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem) : Entity(level)
{
	defineSynchedData();

	life = 0;

	setSize(0.25f, 0.25f);

	setPos(x, y, z);
	heightOffset = 0;

	if(sourceItem){auto copy=sourceItem->copy();copy->count=1;entityData->set(DATA_ID_FIREWORKS_ITEM,copy);}
	int flightCount = 1;
	if (sourceItem != nullptr && sourceItem->hasTag())
	{

		CompoundTag *tag = sourceItem->getTag();
		CompoundTag *compound = tag->getCompound(FireworksItem::TAG_FIREWORKS);
		if (compound != nullptr)
		{
			flightCount += (std::max)(0,(std::min)(3,int(compound->getByte(FireworksItem::TAG_FLIGHT))));
		}
	}
	xd = random->nextGaussian() * .001;
	zd = random->nextGaussian() * .001;
	yd = 0.05;

	lifetime = (SharedConstants::TICKS_PER_SECOND / 2) * flightCount + random->nextInt(6) + random->nextInt(7);
}

void FireworksRocketEntity::lerpMotion(double xd, double yd, double zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	if (xRotO == 0 && yRotO == 0)
	{
		double sd = Mth::sqrt(xd * xd + zd * zd);
		yRotO = yRot = static_cast<float>(atan2(xd, zd) * 180 / PI);
		xRotO = xRot = static_cast<float>(atan2(yd, sd) * 180 / PI);
	}
}

void FireworksRocketEntity::tick()
{
	xOld = x;
	yOld = y;
	zOld = z;
	Entity::tick();

    if(attached || entityData->getInteger(DATA_ID_BOOSTED_ENTITY)>0) {
        auto player=resolveBoostedPlayer();
        if(!player) {if(!level->isClientSide && ++missingOwnerTicks>20)remove();return;}
        missingOwnerTicks=0;
        if(player->isElytraFlying()) {
            Vec3 *look=player->getLookAngle();
            player->xd+=look->x*.1+(look->x*1.5-player->xd)*.5;
            player->yd+=look->y*.1+(look->y*1.5-player->yd)*.5;
            player->zd+=look->z*.1+(look->z*1.5-player->zd)*.5;
        }
        setPos(player->x,player->y,player->z);xd=player->xd;yd=player->yd;zd=player->zd;
    } else {
        xd *= 1.15;zd *= 1.15;yd += .04;move(xd,yd,zd);
    }

	double sd = Mth::sqrt(xd * xd + zd * zd);
	yRot = static_cast<float>(atan2(xd, zd) * 180 / PI);
	xRot = static_cast<float>(atan2(yd, sd) * 180 / PI);

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;

	if (!level->isClientSide )
	{
		if (life == 0)
		{
			level->playEntitySound(shared_from_this(), eSoundType_FIREWORKS_LAUNCH, 3, 1);
		}
	}

	life++;
	if (level->isClientSide && (life % 2) < 2)
	{
		level->addParticle(eParticleType_fireworksspark, x, y - .3, z, random->nextGaussian() * .05, -yd * .5, random->nextGaussian() * .05);
	}
	if (!level->isClientSide && life > lifetime)
	{
		dealExplosionDamage();
		level->broadcastEntityEvent(shared_from_this(), EntityEvent::FIREWORKS_EXPLODE);
		remove();
	}
}

void FireworksRocketEntity::handleEntityEvent(byte eventId)
{
	if (eventId == EntityEvent::FIREWORKS_EXPLODE && level->isClientSide)
	{
		shared_ptr<ItemInstance> sourceItem = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
		CompoundTag *tag = nullptr;
		if (sourceItem != nullptr && sourceItem->hasTag())
		{
			tag = sourceItem->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
		}
		level->createFireworks(x, y, z, xd, yd, zd, tag);
	}
	Entity::handleEntityEvent(eventId);
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putBoolean(L"AttachedToGlider",attached);
	if(attached)tag->putString(L"BoostedUUID",boostedUUID);
	tag->putInt(L"Life", life);
	tag->putInt(L"LifeTime", lifetime);
	shared_ptr<ItemInstance> itemInstance = entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
	if (itemInstance != nullptr)
	{
		CompoundTag *itemTag = new CompoundTag();
		itemInstance->save(itemTag);
		tag->putCompound(L"FireworksItem", itemTag);
	}

}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag *tag)
{
    life=(std::max)(0,tag->getInt(L"Life"));
    lifetime=(std::max)(0,(std::min)(51,tag->getInt(L"LifeTime")));
    attached=tag->getBoolean(L"AttachedToGlider");boostedUUID=tag->getString(L"BoostedUUID");
    if(boostedUUID.size()>64)boostedUUID.clear();boostedPlayer.reset();missingOwnerTicks=0;
    entityData->set(DATA_ID_BOOSTED_ENTITY,0);

	CompoundTag *itemTag = tag->getCompound(L"FireworksItem");
	if (itemTag != nullptr)
	{
		shared_ptr<ItemInstance> fromTag = ItemInstance::fromTag(itemTag);
		if (fromTag != nullptr)
		{
			fromTag->count=1;entityData->set(DATA_ID_FIREWORKS_ITEM, fromTag);
		}
	}
}

float FireworksRocketEntity::getShadowHeightOffs()
{
	return 0;
}

float FireworksRocketEntity::getBrightness(float a)
{
	return Entity::getBrightness(a);
}

int FireworksRocketEntity::getLightColor(float a)
{
	return Entity::getLightColor(a);
}

bool FireworksRocketEntity::isAttackable()
{
	return false;
}

FireworksRocketEntity::FireworksRocketEntity(Level *level,shared_ptr<ItemInstance> item,shared_ptr<Player> player)
    :FireworksRocketEntity(level,player->x,player->y,player->z,item)
{
    attached=true;boostedPlayer=player;boostedUUID=player->getUUID();entityData->set(DATA_ID_BOOSTED_ENTITY,player->entityId);
}
shared_ptr<Player> FireworksRocketEntity::resolveBoostedPlayer()
{
    auto player=boostedPlayer.lock();
    if(player && (player->removed || player->level!=level))player.reset();
    if(!player) {
        if(level->isClientSide)player=dynamic_pointer_cast<Player>(level->getEntity(entityData->getInteger(DATA_ID_BOOSTED_ENTITY)));
        else {
            for(auto &p:level->players)if(p && p->getUUID()==boostedUUID && !p->removed){player=p;break;}
        }
        if(player && player->level==level && !player->removed) {
            boostedPlayer=player;
            if(!level->isClientSide)entityData->set(DATA_ID_BOOSTED_ENTITY,player->entityId);
        } else player.reset();
    }
    return player;
}
void FireworksRocketEntity::dealExplosionDamage()
{
    if(level->isClientSide)return;
    auto item=entityData->getItemInstance(DATA_ID_FIREWORKS_ITEM);
    if(!item || !item->hasTag() || !item->getTag()->contains(FireworksItem::TAG_FIREWORKS,Tag::TAG_Compound))return;
    auto fire=item->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
    if(!fire->contains(FireworksItem::TAG_EXPLOSIONS,Tag::TAG_List))return;
    auto stars=fire->getList(FireworksItem::TAG_EXPLOSIONS);int count=0;
    for(int i=0;i<stars->size();++i)if(stars->get(i)->getId()==Tag::TAG_Compound)++count;
    if(count==0)return;
    float damage=float(5+2*count);auto owner=boostedPlayer.lock();
    if(owner && owner->level==level && !owner->removed)owner->hurt(DamageSource::fireworks,damage);
    auto entities=level->getEntitiesOfClass(typeid(LivingEntity),bb->grow(5,5,5));
    for(auto &entity:*entities) {
        auto living=dynamic_pointer_cast<LivingEntity>(entity);if(!living || living==owner || living->removed)continue;
        double dx=living->x-x,dy=living->y-y,dz=living->z-z,dist2=dx*dx+dy*dy+dz*dz;
        if(dist2>25)continue;
        bool visible=false;
        for(int i=0;i<2;++i)if(level->clip(Vec3::newTemp(x,y,z),Vec3::newTemp(living->x,living->y+living->bbHeight*.5*i,living->z),false,true)==nullptr){visible=true;break;}
        if(visible)living->hurt(DamageSource::fireworks,damage*float(std::sqrt((5-std::sqrt(dist2))/5)));
    }
    delete entities;
}
