#pragma once

#include "Entity.h"

class Player;
class FireworksRocketEntity : public Entity
{
public:
	eINSTANCEOF GetType() { return 	eTYPE_FIREWORKS_ROCKET; }
	static Entity *create(Level *level) { return new FireworksRocketEntity(level); }

private:
	static const int DATA_ID_FIREWORKS_ITEM = 8;
	static const int DATA_ID_BOOSTED_ENTITY = 9;
	weak_ptr<Player> boostedPlayer;
	wstring boostedUUID;
	int missingOwnerTicks=0;
	bool attached=false;
	shared_ptr<Player> resolveBoostedPlayer();
	void dealExplosionDamage();

	int life;
	int lifetime;

	// constructor needed for level loader
public:
	FireworksRocketEntity(Level *level);

protected:
	virtual void defineSynchedData();

public:
	virtual bool shouldRenderAtSqrDistance(double distance);

	FireworksRocketEntity(Level *level,shared_ptr<ItemInstance> item,shared_ptr<Player> player);
	FireworksRocketEntity(Level *level, double x, double y, double z, shared_ptr<ItemInstance> sourceItem);

	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void tick();
	virtual void handleEntityEvent(byte eventId);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual float getShadowHeightOffs();
	virtual float getBrightness(float a);
	virtual int getLightColor(float a);
	virtual bool isAttackable();
};
