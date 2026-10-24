"""Exercise production path, farmer inventory/AI and fossil code in an engine fixture."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
def clean(path):return '\n'.join(l for l in path.read_text().splitlines()if not l.startswith('#include')and not l.startswith('#pragma'))
TEST=r'''
int main(){
 Tile dirt(3);Tile::dirt=&dirt;GrassPathTile path(208);Tile::grassPath=&path;
 assert(!path.solid && path.light==255 && path.shapeY==15.f/16 && !path.isSolidRender() && !path.isCubeShaped() && !path.isSilkTouchable());
 auto a=path.getAABB(nullptr,-2,64,3);assert(a->y1==64.9375 && a->x0==-2);assert(path.getResource(0,nullptr,3)==3);
 Level l;l.blocks[{0,65,0}]=0;l.blocks[{0,64,0}]=208;path.onPlace(&l,0,64,0);assert(l.getTile(0,64,0)==208);l.blocks[{0,65,0}]=1;path.neighborChanged(&l,0,64,0,1);assert(l.getTile(0,64,0)==3);
 l.isClientSide=true;l.blocks[{0,64,0}]=208;path.neighborChanged(&l,0,64,0,1);assert(l.getTile(0,64,0)==208);
 ShovelItem shovel(1,nullptr);auto p=make_shared<Player>();auto tool=make_shared<ItemInstance>(256);
 for(int gate=0;gate<7;++gate){Level w;w.blocks[{0,64,0}]=2;w.blocks[{0,65,0}]=0;p->permitted=gate!=0;if(gate==1)w.blocks[{0,65,0}]=1;if(gate==2)w.blocks[{0,64,0}]=3;if(gate==3)w.fail=true;w.isClientSide=gate==4;int before=tool->damage;bool ok=shovel.useOn(tool,p,&w,0,64,0,gate==5?0:1,0,0,0,gate==6);assert(ok==(gate==4||gate==6));assert(w.writes==0&&tool->damage==before);}
 p->permitted=true;{Level w;w.blocks[{0,64,0}]=2;w.blocks[{0,65,0}]=0;assert(shovel.useOn(tool,p,&w,0,64,0,1,0,0,0,false));assert(w.getTile(0,64,0)==208&&tool->damage==1);}
 FarmerInventory inv;auto stack=make_shared<ItemInstance>(435,70);stack->tag=8;assert(inv.add(stack)==70&&stack->count==70);assert(inv.count(435)==70);assert(inv.consume(435)&&inv.count(435)==69);assert(inv.plantingItem()==435);
 CompoundTag saved;inv.save(&saved);FarmerInventory restored;restored.load(&saved);assert(restored.count(435)==69);CompoundTag savedAgain;restored.save(&savedAgain);auto entry=static_cast<CompoundTag*>(savedAgain.getList(L"Inventory")->get(0));assert(entry->ints[L"tag"]==8);
 assert(inv.add(make_shared<ItemInstance>(1,64))==0);FarmerInventory full;assert(full.add(make_shared<ItemInstance>(391,600))==512);assert(full.add(make_shared<ItemInstance>(392,1))==0);assert(!full.wantsFood());
 {FarmerInventory f;Level w;Villager v;v.level=&w;v.farmInventory=&f;auto d=make_shared<ItemEntity>();d->item=make_shared<ItemInstance>(435,12);w.entities.push_back(d);v.collectFarmItems();v.collectFarmItems();assert(f.count(435)==12&&v.pickups==12&&d->removed);auto blocked=make_shared<ItemEntity>();blocked->item=make_shared<ItemInstance>(391,2);w.entities.push_back(blocked);w.rules.grief=false;v.collectFarmItems();assert(!blocked->removed&&f.count(391)==0);}
 CropTile wheat(59,7),carrot(141,7),potato(142,7),beet(207,3);
 for(int crop:{59,141,142,207}){FarmerInventory f;Level w;Villager v;v.level=&w;v.farmInventory=&f;w.blocks[{0,64,0}]=60;w.blocks[{0,65,0}]=crop;w.data[{0,65,0}]=crop==207?3:7;FarmerHarvestGoal goal(&v);assert(goal.canUse());goal.start();assert(goal.canContinueToUse());goal.tick();assert(w.drops==1&&w.getTile(0,65,0)==0);}
 for(int seed:{295,391,392,435}){FarmerInventory f;f.add(make_shared<ItemInstance>(seed,2));Level w;Villager v;v.level=&w;v.farmInventory=&f;w.blocks[{0,64,0}]=60;w.blocks[{0,65,0}]=0;FarmerHarvestGoal goal(&v);assert(goal.canUse());goal.start();goal.tick();assert(w.getTile(0,65,0)==(seed==295?59:seed==391?141:seed==392?142:207));assert(f.count(seed)==1);}
 for(int gate=0;gate<8;++gate){FarmerInventory f;f.add(make_shared<ItemInstance>(435,2));Level w;Villager v;v.level=&w;v.farmInventory=&f;w.blocks[{0,64,0}]=60;w.blocks[{0,65,0}]=0;if(gate==0)w.isClientSide=true;if(gate==1)w.rules.grief=false;if(gate==2)v.baby=true;if(gate==3)v.trading=true;if(gate==4)v.profession=1;if(gate==5)w.loaded=false;if(gate==6)w.fail=true;if(gate==7)v.nav.reachable=false;FarmerHarvestGoal goal(&v);bool use=goal.canUse();assert(use==(gate>=6));if(use){goal.start();goal.tick();}assert(w.writes==0&&f.count(435)==2);}
 // Mature beetroot is age three. Immature fields and remote targets must not be harvested.
 {FarmerInventory f;Level w;Villager v;v.level=&w;v.farmInventory=&f;w.blocks[{0,64,0}]=60;w.blocks[{0,65,0}]=207;w.data[{0,65,0}]=2;FarmerHarvestGoal goal(&v);assert(!goal.canUse());}
 for(int variant=0;variant<8;++variant)for(int rotation=0;rotation<4;++rotation){Level w,copy;Random r(42),r2(42);assert(FossilFeature::place(&w,&r,-32,16,variant,rotation));FossilFeature::place(&copy,&r2,-32,16,variant,rotation);assert(w.blocks==copy.blocks&&w.data==copy.data);for(auto &b:w.blocks){auto[x,y,z]=b.first;assert(x>=-32&&x<-16&&z>=16&&z<32&&y>=10&&y<65);assert(b.second==16||b.second==216);}}
 for(int gate=0;gate<5;++gate){Level w;Random r(1);if(gate==0)w.isClientSide=true;if(gate==1)w.loaded=false;if(gate==2)w.protectedCells=true;if(gate==3)w.fail=true;if(gate==4)for(int x=0;x<16;++x)for(int y=0;y<65;++y)for(int z=0;z<16;++z)w.blocks[{x,y,z}]=7;assert(!FossilFeature::place(&w,&r,0,0,0,0));assert(w.writes==0);}
 {Level w;Random r;r.fixed=1;assert(!FossilFeature::place(&w,&r,0,0,0,0));assert(w.writes==0);}
 {Level w;Random r;r.fixed=0;FossilFeature::place(&w,&r,0,0,0,1);for(auto d:w.data)assert(d.second==0);}
}
'''
def main():
 source=(ROOT/'tests/overworld_fixture.h').read_text()
 for name in ('FarmerInventory.h','FarmerInventory.cpp','GrassPathTile.h','GrassPathTile.cpp','FarmerHarvestGoal.h','FarmerHarvestGoal.cpp','FossilTemplates.h','FossilFeature.h','FossilFeature.cpp'):source+='\n'+clean(W/name)
 # The native shovel constructor needs the engine's array type; exercise the actual useOn method.
 source+='\nstruct ShovelItem {ShovelItem(int,void*){};bool useOn(shared_ptr<ItemInstance>,shared_ptr<Player>,Level*,int,int,int,int,float,float,float,bool);};\n'
 source+=method('ShovelItem.cpp','bool ShovelItem::useOn(')
 source+=method('Villager.cpp','void Villager::collectFarmItems(')
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'overworld_gameplay',source+TEST)
 village=(W/'VillagePieces.cpp').read_text();assert village.count('case 4:\n\t\treturn Tile::beetroots_Id;')==2
 assert 'Mth::nextInt(random, 2, 7)'not in village
 villager=(W/'Villager.cpp').read_text();assert 'farmInventory->save(tag)'in villager and 'farmInventory->load(tag)'in villager
 assert 'new FarmerHarvestGoal(this)'in villager and 'MIN_MAX_VALUES[391] = pair<int,int>(15, 19)'in villager
 assert 'addItemForTradeIn(newOffers, Item::beetroot_Id'not in villager
 print('Village crop maturity, farmer save/goal/trade wiring: passed')
if __name__=='__main__':main()
