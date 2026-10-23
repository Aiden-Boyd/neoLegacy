"""Compile production city planning/population and shulker combat against fixtures."""
from pathlib import Path
import importlib.util
import shutil
import tempfile
ROOT=Path(__file__).resolve().parents[1]

def method(source,signature):
    start=source.index(signature);opening=source.index('{',start);depth=1;end=opening+1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]+'\n'

TEST=r'''
#include "Shulker.cpp"
#include "EndCityFeature.cpp"
struct TestBullet:ShulkerBullet {using ShulkerBullet::ShulkerBullet;using ShulkerBullet::onHit;};
// Sample the walk surface from template blocks, independently of planner offsets.
double bridgeFloor(const EndCityLayout::Part &p,bool first){
 int index=0;double top=-1;int sampleZ=first?0:p.room->sz-1;
 for(int i=0;i<p.room->runCount;++i){auto run=p.room->runs[i];for(int n=0;n<run.count;++n,++index){
  int x=index%p.room->sx,z=(index/p.room->sx)%p.room->sz,y=index/(p.room->sx*p.room->sz);
  if(x!=2||z!=sampleZ||run.id==0||run.id==198)continue;
  double height=1;
  if(run.id==205&&!(run.data&8))height=.5;
  if(run.id==203&&!(run.data&4))height=((run.data&3)==3?first:!first)?1:.5;
  top=std::max(top,y+height);
 }}assert(top>=0);return p.pos.y+top;
}
void checkConnections(const EndCityLayout::Plan &plan){
 for(size_t i=1;i<plan.parts.size();++i){auto &p=plan.parts[i];auto &previous=plan.parts[i-1];
  bool segment=!strcmp(p.room->name,"bridge_piece")||!strcmp(p.room->name,"bridge_steep_stairs")||!strcmp(p.room->name,"bridge_gentle_stairs");
  if(segment){double from=!strcmp(previous.room->name,"bridge_end")?previous.pos.y+2:bridgeFloor(previous,false);
   double step=bridgeFloor(p,true)-from;assert(step>=-1&&step<=.6);
   auto next=previous.world({0,0,!strcmp(previous.room->name,"bridge_end")?0:previous.room->sz});
   assert(next.x==p.pos.x&&next.z==p.pos.z&&previous.rotation==p.rotation);
  }
  if(!strcmp(p.room->name,"base_floor")&&!strcmp(previous.room->name,"bridge_end")){
   auto door=p.world({4,1,8});auto relative=EndCityLayout::rotate({door.x-previous.pos.x,door.y-previous.pos.y,door.z-previous.pos.z},(4-previous.rotation)&3);
   assert(relative.x==3&&relative.y==2&&relative.z==2);
  }
 }
}
int main(){
 vector<unique_ptr<Tile>> tiles;for(int id=1;id<256;++id)tiles.push_back(unique_ptr<Tile>(new Tile(id)));
 LevelData root;DerivedLevelData wrapper(&root);wrapper.markEndCityChunk(-128,-128);wrapper.markEndCityChunk(127,127);wrapper.markEndCityChunk(128,0);
 assert(root.getEndCityChunks().size()==2&&wrapper.hasEndCityChunk(-128,-128)&&!wrapper.hasEndCityChunk(128,0));
 CompoundTag tag;root.saveCities(&tag);LevelData restored;restored.loadCities(&tag);assert(root.getEndCityChunks()==restored.getEndCityChunks());
 LevelData copied;copied.copyCities(&wrapper);assert(copied.getEndCityChunks()==root.getEndCityChunks());
 Level w;auto shulker=make_shared<Shulker>(&w);shulker->moveTo(.5,10,.5,0,0);w.setTileAndData(0,9,0,201,0,2);
 assert(shulker->getHealth()==30&&shulker->getArmorValue()==20);auto arrow=make_shared<Entity>(&w);
 struct Arrow:Entity{using Entity::Entity;eINSTANCEOF GetType()override{return eTYPE_ARROW;}};
 DamageSource arrowDamage{make_shared<Arrow>(&w)};assert(!shulker->hurt(&arrowDamage,10)&&shulker->getHealth()==30);
 CompoundTag state;state.putByte(L"AttachFace",255);state.putByte(L"Peek",255);shulker->readAdditionalSaveData(&state);assert(shulker->getAttachFace()==0&&shulker->getArmorValue()==20);
 state.putByte(L"Peek",100);shulker->readAdditionalSaveData(&state);assert(shulker->getArmorValue()==0);
 for(int face=0;face<6;++face){state.putByte(L"AttachFace",byte(face));state.putByte(L"Peek",100);shulker->readAdditionalSaveData(&state);w.isClientSide=true;for(int n=0;n<25;++n)shulker->tick();assert(shulker->getAttachFace()==face);double volume=(shulker->bb->x1-shulker->bb->x0)*(shulker->bb->y1-shulker->bb->y0)*(shulker->bb->z1-shulker->bb->z0);assert(abs(volume-2)<.001);}
 w.isClientSide=false;state.putByte(L"AttachFace",0);state.putByte(L"Peek",0);shulker->readAdditionalSaveData(&state);
 w.nearest=make_shared<Player>(&w);w.nearest->moveTo(5,10,.5,0,0);w.entities.push_back(shulker);
 for(int i=0;i<25;++i)shulker->tick();bool fired=false;for(auto e:w.entities)if(e->GetType()==eTYPE_SHULKER_BULLET)fired=true;assert(fired);
 auto victim=make_shared<LivingEntity>(&w);victim->health=20;auto bullet=make_shared<TestBullet>(&w);bullet->owner=shulker;HitResult hit{HitResult::ENTITY,victim};bullet->onHit(&hit);assert(bullet->removed&&victim->health==16&&victim->effectId==25&&victim->effectDuration==200);
 auto blocked=make_shared<TestBullet>(&w);w.isClientSide=true;blocked->onHit(&hit);assert(!blocked->removed&&victim->health==16);w.isClientSide=false;assert(blocked->hurt(&arrowDamage,1)&&blocked->removed);
 auto peaceful=make_shared<TestBullet>(&w);w.difficulty=0;peaceful->tick();assert(peaceful->removed);w.difficulty=2;
 auto saved=make_shared<TestBullet>(&w);CompoundTag bulletTag;bulletTag.putInt(L"Direction",99);bulletTag.putDouble(L"SteerX",NAN);saved->readAdditionalSaveData(&bulletTag);saved->addAdditonalSaveData(&bulletTag);assert(bulletTag.getInt(L"Direction")==1&&bulletTag.getDouble(L"SteerX")==0);
 for(int n=-100;n<100;++n)assert(EndCityLayout::floorDiv(n,20)==int(floor(n/20.)));
 int ships=0;for(int seed=0;seed<100;++seed)for(int rotation=0;rotation<4;++rotation){EndCityLayout::Plan plan(seed,80,0,71,rotation),repeat(seed,80,0,71,rotation);assert(plan.parts.size()==repeat.parts.size()&&plan.parts.size()<100&&plan.valid(2048,256));ships+=plan.hasShip;checkConnections(plan);for(size_t i=0;i<plan.parts.size();++i){auto&a=plan.parts[i];auto&b=repeat.parts[i];assert(a.room==b.room&&a.pos.x==b.pos.x&&a.pos.y==b.pos.y&&a.pos.z==b.pos.z&&a.rotation==b.rotation);for(auto runIndex=0;runIndex<a.room->runCount;++runIndex){auto run=a.room->runs[runIndex];int data=run.data;for(int k=0;k<4;++k)data=EndCityLayout::rotateData(run.id,data,1);assert(data==run.data);}}}assert(ships>0&&ships<400);
 // Generate the same complete city area in opposite chunk orders. The final
 // blocks, metadata, population journal, loot and sentries must agree.
 int cx=0,cz=0;bool chosen=false;
 for(int rx=4;rx<6&&!chosen;++rx)for(int rz=-2;rz<=2&&!chosen;++rz){EndCityLayout::site(42,rx,rz,cx,cz);Random r(42^int64_t(cx)*10387313LL^int64_t(cz)*83492791LL);EndCityLayout::Plan p(42,cx,cz,71,r.nextInt(4));chosen=p.hasShip;}assert(chosen);
 Level a,b;TheEndLevelRandomLevelSource source;
 vector<pair<int,int>> chunks;for(int x=cx-10;x<=cx+10;++x)for(int z=cz-10;z<=cz+10;++z)chunks.push_back({x,z});
 for(auto c:chunks){a.clipWrites=true;a.clipX=c.first;a.clipZ=c.second;EndCityFeature::postProcess(&a,&source,c.first,c.second);}
 reverse(chunks.begin(),chunks.end());for(auto c:chunks){b.clipWrites=true;b.clipX=c.first;b.clipZ=c.second;EndCityFeature::postProcess(&b,&source,c.first,c.second);}
 assert(!a.blocks.empty()&&a.blocks==b.blocks&&a.data==b.data&&a.meta.getEndCityChunks()==b.meta.getEndCityChunks()&&a.entities.size()==b.entities.size());
 int sentries=0,frames=0,loot=0,heads=0,potions=0;for(auto e:a.entities){sentries+=e->GetType()==eTYPE_SHULKER;frames+=e->GetType()==eTYPE_ITEM_FRAME;}
 for(auto e:a.tileEntities){if(auto chest=dynamic_pointer_cast<ChestTileEntity>(e.second)){for(auto item:chest->items)if(item)++loot;}if(auto skull=dynamic_pointer_cast<SkullTileEntity>(e.second)){assert(skull->type==5);++heads;}if(auto stand=dynamic_pointer_cast<BrewingStandTileEntity>(e.second)){assert(stand->items.size()==2&&stand->items[0]->aux==8229);potions+=2;}}
 cerr<<"counts sentries="<<sentries<<" loot="<<loot<<" frames="<<frames<<" heads="<<heads<<" potions="<<potions<<"\n";assert(sentries>0&&loot>0&&frames>0&&heads==frames&&potions==frames*2);
 auto before=a.blocks;size_t entities=a.entities.size();for(auto c:chunks){a.clipWrites=true;a.clipX=c.first;a.clipZ=c.second;EndCityFeature::postProcess(&a,&source,c.first,c.second);}assert(a.blocks==before&&a.entities.size()==entities);
 Level client;client.isClientSide=true;for(auto c:chunks)EndCityFeature::postProcess(&client,&source,c.first,c.second);assert(client.blocks.empty()&&client.entities.empty());
 if(const char *dump=getenv("END_CITY_DUMP")){ofstream out(dump);for(auto cell:a.blocks)if(cell.second)out<<get<0>(cell.first)<<" "<<get<1>(cell.first)<<" "<<get<2>(cell.first)<<" "<<cell.second<<" "<<a.data[cell.first]<<"\n";}
 cout<<"cities/shulkers: passed; sentries="<<sentries<<", ships="<<frames<<", loot stacks="<<loot<<"\n";
}
'''

def main():
    spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    with tempfile.TemporaryDirectory() as temporary:
        folder=Path(temporary);shutil.copyfile(ROOT/'tests/end_city_fixture.h',folder/'stdafx.h')
        headers=['SoundTypes.h','Monster.h','Throwable.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h','net.minecraft.world.phys.h','net.minecraft.world.entity.player.h','net.minecraft.world.entity.ai.attributes.h','net.minecraft.world.level.dimension.h','DamageSource.h','SharedMonsterAttributes.h','PotionItem.h','Difficulty.h','CompoundTag.h','SynchedEntityData.h','MobEffect.h','MobEffectInstance.h','Item.h','ItemInstance.h','TheEndLevelRandomLevelSource.h','EndIslandTerrain.h','LevelData.h','ChestTileEntity.h','BrewingStandTileEntity.h','SkullTileEntity.h','ItemFrame.h','EnchantmentHelper.h','Direction.h','Random.h']
        for name in headers:(folder/name).write_text('#include "stdafx.h"\n')
        for name in ['Shulker.h','Shulker.cpp','EndCityFeature.h','EndCityFeature.cpp','EndCityLayout.h','EndCityTemplates.h']:shutil.copyfile(ROOT/'Minecraft.World'/name,folder/name)
        meta=''
        for name in ['LevelData','DerivedLevelData']:
            source=(ROOT/'Minecraft.World'/(name+'.cpp')).read_text()
            for signature in ['const std::set<int> &'+name+'::getEndCityChunks(', 'bool '+name+'::hasEndCityChunk(', 'void '+name+'::markEndCityChunk(']:meta+=method(source,signature)
        source=(ROOT/'Minecraft.World/LevelData.cpp').read_text()
        start=source.index('intArray cityChunks(');end=source.index('tag->putIntArray(L"EndCityChunks",cityChunks);',start)+len('tag->putIntArray(L"EndCityChunks",cityChunks);')
        meta+='void LevelData::saveCities(CompoundTag *tag){'+source[start:end]+'}\n'
        start=source.index('if(tag->contains(L"EndCityChunks"))');meta+='void LevelData::loadCities(CompoundTag *tag){'+method(source,'if(tag->contains(L"EndCityChunks"))')+'}\n'
        meta+='void LevelData::copyCities(LevelData *copy){'+next(line.strip() for line in source.splitlines() if 'endCityChunks = copy->' in line)+'}\n'
        base.compile_and_run(folder,'end_city_gameplay','#include "stdafx.h"\n'+meta+TEST)

if __name__=='__main__':main()
