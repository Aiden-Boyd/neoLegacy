#include "stdafx.h"
#include "ContentUpdateTiles.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.item.enchantment.h"
#include "DamageSource.h"
#include "IconRegister.h"
#include "SoundTypes.h"
#include "ParticleTypes.h"

MagmaTile::MagmaTile(int id) : Tile(id, Material::stone)
{
    setTicking(true);
    setLightEmission(0.2f);
}
int MagmaTile::getLightColor(LevelSource *level,int x,int y,int z,int tileId) { return 15728880; }
void MagmaTile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
    if (level->isClientSide || entity->isFireImmune() || entity->isSneaking()) return;
    auto living = dynamic_pointer_cast<LivingEntity>(entity);
    if (living && EnchantmentHelper::getEnchantmentLevel(Enchantment::frostWalker->id, living->getCarried(LivingEntity::SLOT_BOOTS)) == 0)
        living->hurt(DamageSource::hotFloor, 1.0f);
}
void MagmaTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (level->isClientSide || level->getTile(x,y,z) != id) return;
    int above = level->getTile(x,y+1,z);
    if (above != Tile::water_Id && above != Tile::calmWater_Id) return;
    if (!level->setTileAndData(x,y+1,z,0,0,Tile::UPDATE_ALL)) return;
    level->playSound(x+0.5,y+0.5,z+0.5,eSoundType_RANDOM_FIZZ,0.5f,2.6f+(random->nextFloat()-random->nextFloat())*0.8f);
    for (int i=0;i<8;++i) level->addParticle(eParticleType_largesmoke,x+random->nextFloat(),y+1.25,z+random->nextFloat(),0,0,0);
}
BoneBlockTile::BoneBlockTile(int id) : RotatedPillarTile(id,Material::stone) { sendTileData(MASK_FACING); }
void BoneBlockTile::registerIcons(IconRegister *icons)
{
    icon = icons->registerIcon(L"bone_block_side");
    iconTop = icons->registerIcon(L"bone_block_top");
}
Icon *BoneBlockTile::getTypeTexture(int type) { return icon; }
int BoneBlockTile::getSpawnResourcesAuxValue(int data) { return 0; }
int BoneBlockTile::getType(int data) { return 0; }
