"""Compile production food inventory, social tick and breeding goal."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1]
def clean(p):return '\n'.join(l for l in p.read_text().splitlines() if not l.startswith('#include') and not l.startswith('#pragma'))
def main():
 s=(ROOT/'tests/villager_food_fixture.h').read_text()
 for p in ['FarmerInventory.h','FarmerInventory.cpp','MakeLoveGoal.h','MakeLoveGoal.cpp']:s+='\n'+clean(ROOT/'Minecraft.World'/p)
 for sig in ['bool Villager::isWilling(', 'void Villager::setWilling(', 'void Villager::shareFood(', 'void Villager::collectFarmItems(']:s+='\n'+method('Villager.cpp',sig)
 s+=TEST
 with tempfile.TemporaryDirectory() as d:compile_and_run(Path(d),'villager_food',s)
TEST=r'''
int main(){
 for(int id:{297,391,392,434}){int cost=id==297?3:12;FarmerInventory f;f.add(make_shared<ItemInstance>(id,cost-1));assert(!f.prepareBreeding());f.add(make_shared<ItemInstance>(id,1));assert(f.prepareBreeding()&&f.count(id)==0);assert(f.prepareBreeding());CompoundTag save;f.save(&save);FarmerInventory copy;copy.load(&save);assert(copy.willing&&copy.count(id)==0);}
 {FarmerInventory f;f.add(make_shared<ItemInstance>(296,64));assert(!f.prepareBreeding());auto gift=f.share(true,false);assert(gift->id==297&&gift->count==10&&f.count(296)==64);f.share(true,true);assert(f.count(296)==34);assert(!f.share(false,false));}
 for(int id:{297,391,392,434}){FarmerInventory f;int n=id==297?6:24;auto item=make_shared<ItemInstance>(id,n);item->tag=17;f.add(item);auto gift=f.share(false,false);assert(gift&&gift->count==n/2&&gift->tag==17);f.share(false,true);assert(f.count(id)==n/2&&item->count==n);}
 Level w;FarmerInventory a,b;auto v=make_shared<Villager>(),mate=make_shared<Villager>();v->level=mate->level=&w;v->farmInventory=&a;mate->farmInventory=&b;mate->profession=1;mate->x=2;v->random.forced=0;
 a.add(make_shared<ItemInstance>(296,18));w.entities={v,mate};v->shareFood();assert(v->foodShareDelay==10);for(int i=0;i<10;++i)v->shareFood();assert(a.count(296)==9);auto drop=dynamic_pointer_cast<ItemEntity>(w.entities.back());assert(drop&&drop->item->id==297&&drop->item->count==3&&drop->throwTime==10);
 drop->throwTime=0;mate->collectFarmItems();assert(b.count(297)==3&&drop->removed);assert(mate->isWilling(true)&&b.count(297)==0);
 a.add(make_shared<ItemInstance>(297,6));v->foodShareDelay=1;v->foodShareTarget=mate;int bread=a.count(297),wheat=a.count(296);w.spawn=false;v->shareFood();assert(a.count(297)==bread&&a.count(296)==wheat);w.spawn=true;
 // No sharing across walls, while trading, on client, or after a target dies.
 for(int gate=0;gate<4;++gate){v->foodShareDelay=1;v->foodShareTarget=mate;int food=a.count(297);if(gate==0)v->visible=false;if(gate==1)v->trading=true;if(gate==2)w.isClientSide=true;if(gate==3)mate->alive=false;v->shareFood();assert(a.count(297)==food&&v->foodShareDelay==0);v->visible=true;v->trading=false;w.isClientSide=false;mate->alive=true;}
 // Empty/underfed adults cannot breed; food prepares and persists willingness.
 w.closest=mate;MakeLoveGoal goal(v.get());w.collection.value->doors=0;assert(!goal.canUse());w.collection.value->doors=10;assert(goal.canUse());goal.start();mate->setInLove(true);assert(goal.canContinueToUse());
 // Village capacity, village cooldown, trade, dead and age gates.
 w.collection.value->population=3;assert(!goal.canContinueToUse());w.collection.value->population=2;
 w.collection.value->timer=false;assert(!goal.canContinueToUse());w.collection.value->timer=true;
 mate->age=1;assert(!goal.canContinueToUse());mate->age=0;mate->trading=true;assert(!goal.canContinueToUse());mate->trading=false;
 // Failed child spawn and null offspring retain parent state without crashing.
 mate->x=v->x;w.spawn=false;for(int i=0;i<300;++i)goal.tick();assert(v->age==0&&mate->age==0&&a.willing&&b.willing);
 goal.stop();assert(goal.canUse());goal.start();v->childAllowed=false;for(int i=0;i<300;++i)goal.tick();assert(v->age==0&&mate->age==0);v->childAllowed=true;
 goal.stop();assert(goal.canUse());goal.start();w.spawn=true;int before=w.births;for(int i=0;i<300;++i)goal.tick();assert(w.births==before+1&&v->age==6000&&mate->age==6000&&!a.willing&&!b.willing);auto child=dynamic_pointer_cast<Villager>(w.entities.back());assert(child&&child->age==-24000);goal.tick();assert(w.births==before+1);
}
'''
if __name__=='__main__':main()
