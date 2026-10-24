#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystal.h"
#include "DamageSource.h"
#include "EndFight.h"
#include "com.mojang.nbt.h"
#include "Dimension.h"



void EnderCrystal::_init(Level *level)
{		
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	life=MAX_LIFE;
	this->defineSynchedData();

	blocksBuilding = true;
	noCulling = true; // The beam extends outside the crystal hitbox; do not cull it with that box.
	setSize(2.0f, 2.0f);
	heightOffset = bbHeight / 2.0f;
	life = MAX_LIFE;

	time = random->nextInt(100000);
}

EnderCrystal::EnderCrystal(Level *level) : Entity( level )
{
	_init(level);

}

EnderCrystal::EnderCrystal(Level *level, double x, double y, double z) : Entity( level )
{
	_init(level);
	setPos(x, y, z);

}

bool EnderCrystal::makeStepSound()
{
	return false;
}

void EnderCrystal::defineSynchedData()
{
	entityData->define(DATA_REMAINING_LIFE, life);
	entityData->define(DATA_SHOW_BOTTOM,byte(1));
	entityData->define(DATA_BEAM,byte(0));
	entityData->define(DATA_BEAM_X,0);entityData->define(DATA_BEAM_Y,128);entityData->define(DATA_BEAM_Z,0);
}

void EnderCrystal::tick()
{
	xo = x;
	yo = y;
	zo = z;
	time++;

	entityData->set(DATA_REMAINING_LIFE, life);

	// Don't set the tile directly on the client, as this can end up in the updatesToReset queue in the MultiPlayerLevel, and the perpetually end up removing/adding these fire tiles causing timing
	// glitches from the lighting changes requried.
	if (!level->isClientSide && level->dimension->id==1)
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(y);
		int zt = Mth::floor(z);
		if (level->isEmptyTile(xt, yt, zt))
		{
			level->setTileAndUpdate(xt, yt, zt, Tile::fire_Id);
		}
	}
}


void EnderCrystal::addAdditonalSaveData(CompoundTag *tag)
{
    tag->putBoolean(L"ShowBottom",shouldShowBottom());
    if(hasBeamTarget()){auto beam=new CompoundTag();beam->putInt(L"X",getBeamX());beam->putInt(L"Y",getBeamY());beam->putInt(L"Z",getBeamZ());tag->putCompound(L"BeamTarget",beam);}
}

void EnderCrystal::readAdditionalSaveData(CompoundTag *tag)
{
    setShowBottom(!tag->contains(L"ShowBottom") || tag->getBoolean(L"ShowBottom"));
    clearBeamTarget();
    if(tag->contains(L"BeamTarget",Tag::TAG_Compound)) {auto beam=tag->getCompound(L"BeamTarget");int x=beam->getInt(L"X"),y=beam->getInt(L"Y"),z=beam->getInt(L"Z");if(x>=-2048&&x<=2048&&z>=-2048&&z<=2048&&y>=0&&y<=255)setBeamTarget(x,y,z);}
}


float EnderCrystal::getShadowHeightOffs()
{
	return 0;
}

bool EnderCrystal::isPickable()
{
	return true;
}

bool EnderCrystal::hurt(DamageSource *source, float damage)
{
	if (isInvulnerable()) return false;

	// 4J-PB - if the owner of the source is the enderdragon, then ignore it (where the dragon's fireball hits an endercrystal)
	if ( source->getEntity() != nullptr && source->getEntity()->instanceof(eTYPE_ENDERDRAGON) )
	{
		return false;
	}

	if (!removed && !level->isClientSide)
	{
		life = 0;
		if (life <= 0)
		{
			remove();
			EndFight::crystalDestroyed(level,this);
			if (!level->isClientSide)
			{
				if(!source->isExplosion())level->explode(nullptr, x, y, z, 6, true);

				vector<shared_ptr<Entity> > entities = level->getAllEntities();
				shared_ptr<EnderDragon> dragon = nullptr;
				for ( auto& e : entities )
				{
					dragon = dynamic_pointer_cast<EnderDragon>(e);
					if ( dragon )
					{
						dragon->handleCrystalDestroyed(source);
						break;
					}
				}
			}
		}
	}
	return true;
}

void EnderCrystal::setShowBottom(bool show) {entityData->set(DATA_SHOW_BOTTOM,byte(show?1:0));}
bool EnderCrystal::shouldShowBottom() {return entityData->getByte(DATA_SHOW_BOTTOM)!=0;}
void EnderCrystal::setBeamTarget(int x,int y,int z) {entityData->set(DATA_BEAM_X,x);entityData->set(DATA_BEAM_Y,y);entityData->set(DATA_BEAM_Z,z);entityData->set(DATA_BEAM,byte(1));}
void EnderCrystal::clearBeamTarget() {entityData->set(DATA_BEAM,byte(0));}
bool EnderCrystal::hasBeamTarget() {return entityData->getByte(DATA_BEAM)!=0;}
int EnderCrystal::getBeamX() {return entityData->getInteger(DATA_BEAM_X);}
int EnderCrystal::getBeamY() {return entityData->getInteger(DATA_BEAM_Y);}
int EnderCrystal::getBeamZ() {return entityData->getInteger(DATA_BEAM_Z);}
bool EnderCrystal::shouldRenderAtSqrDistance(double distance) {return hasBeamTarget() || Entity::shouldRenderAtSqrDistance(distance);}
