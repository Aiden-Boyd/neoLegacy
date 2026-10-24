"""Compile production gameplay for the Nether, bone and beetroot candidate."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
TEST=r'''
#include "ContentUpdateTiles.cpp"
#include "RotatedPillarTile.cpp"
#include "CropTile.cpp"
#include "BeetrootTile.cpp"
#include "SeedItem.cpp"
#include "BowlFoodItem.cpp"
int main(){
 Enchantment frost;Enchantment::frostWalker=&frost;MagmaTile magma(213);Tile::magma=&magma;assert(magma.emission==.2f&&magma.ticking&&magma.getLightColor(nullptr,0,0,0)==15728880);
 auto player=make_shared<Player>();Level world;player->level=&world;world.blocks[{0,63,0}]=213;
 player->updateHotFloor();assert(player->health==19);player->updateHotFloor();assert(player->health==18); // actual engine adds its normal hurt cooldown
 for(int gate=0;gate<5;++gate){auto e=make_shared<Player>();e->level=&world;if(gate==0)e->immune=true;if(gate==1)e->sneaking=true;if(gate==2){e->equipment[1]=make_shared<ItemInstance>(313);e->equipment[1]->frost=true;}if(gate==3)e->fireResistance=true;if(gate==4)e->onGround=false;e->updateHotFloor();assert(e->health==20);}
 {auto e=make_shared<Player>();e->level=&world;e->equipment[0]=make_shared<ItemInstance>(313);e->equipment[0]->frost=true;e->updateHotFloor();assert(e->health==19);}
 {auto e=make_shared<Player>();e->level=&world;world.isClientSide=true;e->updateHotFloor();magma.stepOn(&world,0,63,0,e);assert(e->health==20);world.isClientSide=false;}
 magma.stepOn(&world,0,63,0,make_shared<Entity>());assert(DamageSource::hotFloor->fire);
 for(int above:{8,9,1,11}){Level l;Random r;l.blocks[{0,63,0}]=213;l.blocks[{0,64,0}]=above;magma.tick(&l,0,63,0,&r);assert(l.getTile(0,64,0)==((above==8||above==9)?0:above));assert(l.particles==((above==8||above==9)?8:0));}
 {Level l;Random r;l.blocks[{0,63,0}]=213;l.blocks[{0,64,0}]=9;l.failWrites=true;magma.tick(&l,0,63,0,&r);assert(l.getTile(0,64,0)==9&&l.particles==0);l.isClientSide=true;magma.tick(&l,0,63,0,&r);assert(l.sounds==0);}
 {auto mob=make_shared<Mob>();mob->level=&world;assert(!mob->canSpawn());mob->immune=true;assert(mob->canSpawn());world.blocks[{0,63,0}]=1;mob->immune=false;assert(mob->canSpawn());}
 BoneBlockTile bone(216);IconRegister icons;bone.registerIcons(&icons);assert(bone.sentMask==12);
 for(int face=0;face<6;++face){int data=bone.getPlacedOnFaceDataValue(nullptr,0,0,0,face,.5,.5,.5,0);for(int side=0;side<6;++side)assert(bone.getTexture(side,data)->name==(side/2==face/2?L"bone_block_top":L"bone_block_side"));assert(bone.getSpawnResourcesAuxValue(data)==0&&bone.getType(data)==0);}
 BeetrootTile beet(207);beet.registerIcons(&icons);assert(beet.getMaxAge()==3);assert(beet.cloneTileId(nullptr,0,0,0)==435);
 for(int age=0;age<4;++age){Level l;l.blocks[{0,63,0}]=60;l.blocks[{0,64,0}]=207;l.data[{0,64,0}]=age;beet.updateShape(&l,0,64,0);assert(beet.y1==(age+1)*.125f);assert(beet.getTexture(0,age)->name==L"beetroots_stage_"+to_wstring(age));assert(beet.getResource(age,nullptr,0)==(age==3?434:435));beet.spawnResources(&l,0,64,0,age,1,0);assert(l.drops.size()==(age==3?4u:1u));assert(l.drops[0]->id==(age==3?434:435));}
 {Level l;l.blocks[{0,63,0}]=60;l.blocks[{0,64,0}]=207;l.data[{0,64,0}]=3;beet.spawnResources(&l,0,64,0,3,1,2);assert(l.drops.size()==6);l.drops.clear();l.rng.choice=5;beet.spawnResources(&l,0,64,0,3,1,0);assert(l.drops.size()==1&&l.drops[0]->id==434);l.drops.clear();l.isClientSide=true;beet.spawnResources(&l,0,64,0,3,1,0);assert(l.drops.empty());}
 // Bone meal increases 0/1 stage, clamps at 3; default crops still use 2-5 and max 7.
 struct Wheat:CropTile {Wheat():CropTile(59){}}wheat;
 for(int random=0;random<4;++random){Level l;l.rng.choice=random;l.data[{0,64,0}]=1;beet.growCrops(&l,0,64,0);assert(l.getData(0,64,0)==1+(2+random)/3);l.data[{0,64,0}]=1;wheat.growCrops(&l,0,64,0);assert(l.getData(0,64,0)==3+random);}
 assert(wheat.getMaxAge()==7&&wheat.getResource(7,nullptr,0)==296&&wheat.getResource(6,nullptr,0)==295);
 {Level l;l.blocks[{0,63,0}]=60;l.blocks[{0,64,0}]=207;l.data[{0,64,0}]=2;l.rng.values.push(1);l.rng.values.push(0);beet.tick(&l,0,64,0,l.random);assert(l.getData(0,64,0)==3);l.rng.values.push(1);l.rng.values.push(0);beet.tick(&l,0,64,0,l.random);assert(l.getData(0,64,0)==3);}
 {Level l;l.blocks[{0,63,0}]=60;l.blocks[{0,64,0}]=207;l.brightness=0;l.rng.values.push(1);l.rng.values.push(0);beet.tick(&l,0,64,0,l.random);assert(l.getData(0,64,0)==0);l.isClientSide=true;int before=l.writes;beet.tick(&l,0,64,0,l.random);assert(l.writes==before);}
 {Level l;l.blocks[{0,64,0}]=207;l.rng.choice=0;beet.tick(&l,0,64,0,l.random);assert(l.getTile(0,64,0)==0);}
 // Planting uses native permissions, farmland/top-face checks, preview and write success.
 for(int crop:{207,59,115}){int land=crop==115?88:60;SeedItem seed(179,crop,land);Level l;l.blocks[{0,63,0}]=land;auto p=make_shared<Player>();auto stack=make_shared<ItemInstance>(435,2);assert(seed.useOn(stack,p,&l,0,63,0,1,.5,.5,.5,true)&&stack->count==2&&l.writes==0);l.failWrites=true;assert(!seed.useOn(stack,p,&l,0,63,0,1,.5,.5,.5)&&stack->count==2);l.failWrites=false;p->allowed=false;assert(!seed.useOn(stack,p,&l,0,63,0,1,.5,.5,.5));p->allowed=true;assert(!seed.useOn(stack,p,&l,0,63,0,2,.5,.5,.5));assert(seed.useOn(stack,p,&l,0,63,0,1,.5,.5,.5)&&stack->count==1&&l.getTile(0,64,0)==crop);assert(!seed.useOn(stack,p,&l,0,63,0,1,.5,.5,.5));}
 Item bowl(25);Item::bowl=&bowl;BowlFoodItem soup(180,6);assert(soup.nutrition==6&&soup.maxStackSize==1);auto portion=make_shared<ItemInstance>(436);auto result=soup.useTimeDepleted(portion,&world,player);assert(result->id==281&&result->count==1&&portion->count==0);
 Pig pig;Chicken chicken;assert(pig.isFood(make_shared<ItemInstance>(434))&&pig.isFood(make_shared<ItemInstance>(392))&&!pig.isFood(make_shared<ItemInstance>(435))&&!pig.isFood(nullptr));assert(chicken.isFood(make_shared<ItemInstance>(435))&&!chicken.isFood(make_shared<ItemInstance>(434))&&!chicken.isFood(nullptr));
}
'''
def main():
 with tempfile.TemporaryDirectory()as tmp:
  f=Path(tmp);(f/'stdafx.h').write_bytes((ROOT/'tests/content_update_fixture.h').read_bytes())
  headers=['net.minecraft.world.level.tile.h','net.minecraft.h','net.minecraft.world.h','net.minecraft.world.level.h','net.minecraft.world.level.material.h','net.minecraft.world.entity.h','net.minecraft.world.phys.h','net.minecraft.world.item.enchantment.h','DamageSource.h','IconRegister.h','SoundTypes.h','ParticleTypes.h','net.minecraft.world.entity.item.h','net.minecraft.world.item.h','net.minecraft.world.entity.player.h','Tile.h','Bush.h','Material.h','FoodItem.h','Item.h','ItemInstance.h']
  for name in headers:(f/name).write_text('#include "stdafx.h"\n')
  for name in ['ContentUpdateTiles.h','ContentUpdateTiles.cpp','BeetrootTile.h','BeetrootTile.cpp','CropTile.h','CropTile.cpp','RotatedPillarTile.h','RotatedPillarTile.cpp','SeedItem.h','SeedItem.cpp','BowlFoodItem.h','BowlFoodItem.cpp']:(f/name).write_bytes((W/name).read_bytes())
  bodies='\n'.join(method(file,sig)for file,sig in [('LivingEntity.cpp','void LivingEntity::updateHotFloor'),('Mob.cpp','bool Mob::canSpawn()'),('Pig.cpp','bool Pig::isFood'),('Chicken.cpp','bool Chicken::isFood')])
  damage=next(s for s in(W/'DamageSource.cpp').read_text().splitlines()if s.startswith('DamageSource *DamageSource::hotFloor ='))
  compile_and_run(f,'content_gameplay','#include "stdafx.h"\n'+damage+'\n'+bodies+'\n'+TEST)
 from content_recipe_regression import verify_recipes
 verify_recipes()
 from content_generation_regression import verify_generation
 verify_generation()
 verify_integration()
 from content_assets_regression import verify_assets
 verify_assets()

def verify_integration():
 crop=(W/'DyePowderItem.cpp').read_text();assert 'tile == Tile::beetroots_Id' in crop and '->getMaxAge()) return false;'in crop
 assert 'tile == Tile::beetroots_Id'in(W/'FarmTile.cpp').read_text()
 item=(W/'Item.cpp').read_text();assert 'FoodItem(beetroot_Id-256,1,FoodConstants::FOOD_SATURATION_NORMAL,false)'in item
 assert 'SeedItem(beetrootSeeds_Id-256,Tile::beetroots_Id,Tile::farmland_Id)'in item
 assert 'BowlFoodItem(beetrootSoup_Id-256,6)'in item
 city=(W/'EndCityFeature.cpp').read_text();assert '{435,5,1,10,false}'in city
 for name in ['magma_Id','netherWartBlock_Id','redNetherBrick_Id','boneBlock_Id','beetroot_Id','beetrootSeeds_Id','beetrootSoup_Id']:assert 'ITEM('+('Tile::'if name.endswith('Block_Id')or name in ['magma_Id','redNetherBrick_Id']else'Item::')+name+')'in(ROOT/'Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp').read_text()
 assert 'updateHotFloor();'in method('LivingEntity.cpp','void LivingEntity::tick()')
 for path in ['Minecraft.Client/ServerPlayer.cpp','Minecraft.Client/ClientConnection.cpp']:
  source=(ROOT/path).read_text();assert 'case ChatPacket::e_ChatDeathHotFloor:'in source and 'case ChatPacket::e_ChatDeathHotFloorPlayer:'in source
 print('Nether/beetroot UI, item, bone meal, farmland, city loot and chat integration: passed')
if __name__=='__main__':main()
