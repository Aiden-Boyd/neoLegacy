"""Compile production ritual, crystal, item, pillar and dragon death methods."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
def clean(name):return '\n'.join(l for l in (W/name).read_text().splitlines()if not l.startswith('#include')and not l.startswith('#pragma'))
ENGINE=r'''
void Level::explode(shared_ptr<Entity> source,double x,double y,double z,float r,bool blocks){
 explosions.emplace_back(x,y,z,r,blocks);DamageSource damage;damage.source=source;damage.explosion=true;
 auto copy=entities;for(auto e:copy)if(auto c=dynamic_pointer_cast<EnderCrystal>(e);c&&!c->removed){double dx=c->x-x,dy=c->y-y,dz=c->z-z;if(dx*dx+dy*dy+dz*dz<4*r*r)c->hurt(&damage,1);}
}
bool Level::addEntity(shared_ptr<Entity> e){if(failAdd)return false;entities.push_back(e);if(auto orb=dynamic_pointer_cast<ExperienceOrb>(e))xp+=orb->value;return true;}
vector<shared_ptr<Entity>>*Level::getEntitiesOfClass(const type_info&,AABB*b){auto out=new vector<shared_ptr<Entity>>;for(auto e:entities)if(dynamic_pointer_cast<EnderCrystal>(e)&&!e->removed&&e->bb->intersects(b))out->push_back(e);return out;}
'''
TEST=r'''
shared_ptr<Player> player(Level &l){auto p=make_shared<Player>(&l);p->setPos(10,65,10);l.players.push_back(p);return p;}
void arena(Level &l){player(l);for(int x=-2;x<=2;++x)for(int z=-2;z<=2;++z)if((x||z)&&x*x+z*z<=6)l.blocks[{x,64,z}]=119;l.blocks[{0,64,0}]=7;for(int i=0;i<8;++i)l.blocks[{pillarX[i],76+i*3,pillarZ[i]}]=7;}
shared_ptr<EnderCrystal> putCrystal(Level&l,int x,int z){auto c=make_shared<EnderCrystal>(&l,x+.5,65,z+.5);c->setShowBottom(false);l.addEntity(c);return c;}
void four(Level&l){putCrystal(l,0,-3);putCrystal(l,0,3);putCrystal(l,-3,0);putCrystal(l,3,0);}
int livingCount(Level&l){int n=0;for(auto e:l.entities)if(dynamic_pointer_cast<EnderDragon>(e)&&!e->removed)++n;return n;}
void tick(Level&l,int n=1){for(int i=0;i<n;++i){++l.time;EndFight::tick(&l);}}
void reload(Level&l){CompoundTag saved;l.data.fight.save(&saved);EndFightState restored;restored.load(&saved);l.data.fight=restored;auto old=l.entities;l.entities.clear();for(auto e:old){if(e->removed)continue;if(auto c=dynamic_pointer_cast<EnderCrystal>(e)){auto tag=c->save();auto copy=make_shared<EnderCrystal>(&l,c->x,c->y,c->z);copy->load(&tag);l.entities.push_back(copy);}else l.entities.push_back(e);}}
int main(){
 // Item placement: correct supports, permission, two replaceable blocks, physical occupancy,
 // authority/preview, Creative retention, failed entity spawn, glint and no non-End fire.
 EndCrystalItem item(170);assert(item.isFoil(nullptr));
 for(int gate=0;gate<9;++gate){Level l;auto p=player(l);auto stack=make_shared<ItemInstance>();l.blocks[{5,64,5}]=gate==0?1:49;if(gate==1)p->allowed=false;if(gate==2)l.blocks[{5,65,5}]=1;if(gate==3)l.blocks[{5,66,5}]=1;if(gate==4){auto e=make_shared<Entity>(&l);e->setPos(5.5,65,5.5);l.addEntity(e);}if(gate==5)l.loaded=false;if(gate==6)l.failAdd=true;if(gate==7)l.isClientSide=true;bool ok=item.useOn(stack,p,&l,5,64,5,1,0,0,0,gate==8);assert(ok==(gate>=7));assert(stack->count==1);assert(l.entities.size()==(gate==4?1u:0u));}
 for(bool creative:{false,true}){Level l;auto p=player(l);p->abilities.instabuild=creative;l.dim.id=0;l.blocks[{5,64,5}]=7;l.blocks[{5,65,5}]=31;auto stack=make_shared<ItemInstance>();assert(item.useOn(stack,p,&l,5,64,5,1,0,0,0,false));assert(stack->count==(creative?1:0));auto c=dynamic_pointer_cast<EnderCrystal>(l.entities.back());assert(c&&!c->shouldShowBottom());l.blocks[{5,65,5}]=0;c->tick();assert(l.getTile(5,65,5)==0);l.dim.id=1;c->tick();assert(l.getTile(5,65,5)==51);l.blocks[{5,65,5}]=1;c->tick();assert(l.getTile(5,65,5)==1);}
 {Level l;auto c=putCrystal(l,0,0);assert(c->life==5&&c->data.getInteger(8)==5&&c->noCulling);c->setBeamTarget(-40,100,28);c->setInvulnerable(true);auto saved=c->save();auto copy=make_shared<EnderCrystal>(&l);copy->load(&saved);assert(!copy->shouldShowBottom()&&copy->hasBeamTarget()&&copy->getBeamX()==-40&&copy->getBeamY()==100&&copy->getBeamZ()==28&&copy->isInvulnerable());assert(copy->shouldRenderAtSqrDistance(1e9));copy->clearBeamTarget();assert(!copy->shouldRenderAtSqrDistance(1e9));CompoundTag old;old.putString(L"UUID",L"old");copy->load(&old);assert(copy->shouldShowBottom()&&!copy->hasBeamTarget());}
 // Migration and trigger require four distinct crystals, an active exit and no dragon.
 for(int gate=0;gate<5;++gate){Level l;arena(l);four(l);if(gate==0)l.isClientSide=true;if(gate==1)l.dim.id=0;if(gate==2)l.loaded=false;if(gate==3)l.entities.back()->remove();if(gate==4)l.addEntity(make_shared<EnderDragon>(&l));assert(!EndFight::tryRespawn(&l));assert(l.data.fight.phase==-1);}
 {Level l;arena(l);four(l);assert(EndFight::tryRespawn(&l));assert(l.data.fight.previouslyKilled&&l.data.fight.dragonKilled&&l.data.fight.phase==0);assert(l.getTile(1,64,0)==0);assert(!EndFight::tryRespawn(&l));tick(l);assert(l.data.fight.phase==1);assert(dynamic_pointer_cast<EnderCrystal>(l.entities.front())->hasBeamTarget());int phase=l.data.fight.phase,t=l.data.fight.ticks;l.loaded=false;tick(l,80);assert(l.data.fight.phase==phase&&l.data.fight.ticks==t);l.loaded=true;l.players.clear();tick(l,80);assert(l.data.fight.ticks==t);}
 // Full ritual with a reload every seven ticks: every pillar exactly once; one dragon;
 // four ritual crystals consumed, eight destructible normal crystals remain, portal closed.
 {Level l;arena(l);four(l);assert(EndFight::tryRespawn(&l));for(int t=0;t<700&&livingCount(l)==0;++t){tick(l);if(t%7==0)reload(l);}assert(livingCount(l)==1&&l.data.fight.phase==-1&&!l.data.fight.dragonKilled);assert(l.data.fight.previouslyKilled&&!l.data.fight.dragonId.empty());int pillars=0,ritualExplosions=0,liveCrystals=0;for(auto e:l.explosions){if(get<3>(e)==5)++pillars;else if(get<3>(e)==6){++ritualExplosions;assert(!get<4>(e));}}assert(pillars==8&&ritualExplosions==4);for(auto e:l.entities)if(auto c=dynamic_pointer_cast<EnderCrystal>(e);c&&!c->removed){++liveCrystals;assert(!c->isInvulnerable()&&!c->hasBeamTarget()&&c->shouldShowBottom());}assert(liveCrystals==8);assert(l.getTile(1,64,0)==0);tick(l,100);assert(livingCount(l)==1);for(int i=0;i<8;++i)assert(l.getTile(pillarX[i],76+i*3,pillarZ[i])==7);assert(l.getTile(0,94,-40)==7&&l.getTile(2,97,-38)==101);}
 // Destroy one ritual crystal in each phase, including the final spawn boundary.
 for(int phase=0;phase<=4;++phase){Level l;arena(l);four(l);assert(EndFight::tryRespawn(&l));for(int t=0;t<700&&l.data.fight.phase!=phase;++t)tick(l);assert(l.data.fight.phase==phase);auto c=crystalById(&l,l.data.fight.crystals[0]);assert(c);DamageSource source;c->hurt(&source,1);assert(l.data.fight.phase==-1&&l.data.fight.dragonKilled&&l.getTile(1,64,0)==119);for(auto e:l.entities)if(auto other=dynamic_pointer_cast<EnderCrystal>(e);other&&!other->removed)assert(!other->hasBeamTarget()&&!other->isInvulnerable());tick(l,30);assert(livingCount(l)==0);}
 // Missing crystal on reload aborts. Failed spawn retains crystals and retries once.
 {Level l;arena(l);four(l);EndFight::tryRespawn(&l);tick(l,20);l.entities.front()->remove();reload(l);tick(l);assert(l.data.fight.phase==-1&&l.getTile(1,64,0)==119);}
 {Level l;arena(l);four(l);EndFight::tryRespawn(&l);for(int t=0;t<700&&l.data.fight.phase!=4;++t)tick(l);l.failAdd=true;tick(l,4);assert(livingCount(l)==0&&l.data.fight.phase==4);l.failAdd=false;reload(l);tick(l);assert(livingCount(l)==1);reload(l);tick(l,4);assert(livingCount(l)==1);}
 {Level l;arena(l);four(l);assert(EndFight::tryRespawn(&l));l.data.fight.crystals[1]=l.data.fight.crystals[0];reload(l);tick(l);assert(l.data.fight.phase==-1&&l.getTile(1,64,0)==119&&livingCount(l)==0);}
 {Level l;arena(l);four(l);for(auto e:l.entities)e->setUUID(L"duplicate");assert(!EndFight::tryRespawn(&l));}
 {Level l;arena(l);four(l);l.blocks[{40,76,0}]=0;l.blocks[{40,75,0}]=49;assert(EndFight::tryRespawn(&l));assert(l.data.fight.pillarHeights[0]==76);}
 {Level l;arena(l);four(l);assert(EndFight::tryRespawn(&l));l.data.fight.setPhase(4);auto existing=make_shared<EnderDragon>(&l);l.addEntity(existing);reload(l);tick(l);assert(livingCount(l)==1&&l.data.fight.dragonId==existing->getUUID()&&l.data.fight.phase==-1);}
 // Crystal explosion sources don't recursively create more explosions. Dragon/invulnerable attacks fail.
 {Level l;auto c=putCrystal(l,0,0);DamageSource source;source.explosion=true;c->hurt(&source,1);assert(c->removed&&l.explosions.empty());c=putCrystal(l,1,0);c->setInvulnerable(true);assert(!c->hurt(&source,1)&&!c->removed);c->setInvulnerable(false);source.source=make_shared<EnderDragon>(&l);assert(!c->hurt(&source,1)&&!c->removed);}
 // Actual dragon death method: XP and egg exactly once, including death-animation saves.
 for(int repeat=0;repeat<2;++repeat)for(bool loot:{false,true}){Level l;player(l);l.rules.loot=loot;l.data.fight.initialized=true;l.data.fight.previouslyKilled=repeat;auto dragon=make_shared<EnderDragon>(&l);l.addEntity(dragon);for(int t=0;t<200;++t){dragon->tickDeath();if(t==172){auto saved=dragon->save();auto copy=make_shared<EnderDragon>(&l);copy->load(&saved);copy->setPos(dragon->x,dragon->y,dragon->z);dragon->remove();l.addEntity(copy);dragon=copy;}}assert(dragon->removed&&l.xp==(loot?(repeat?500:12000):0)&&l.eggs==(repeat?0:1));assert(l.data.gateways==1&&l.data.fight.dragonKilled&&l.data.fight.previouslyKilled);assert(!EndFight::dragonDefeated(&l,dragon.get()));}
 {Level l;player(l);auto dragon=make_shared<EnderDragon>(&l);l.addEntity(dragon);l.data.fight.initialized=true;l.data.fight.previouslyKilled=true;l.data.fight.dragonKilled=true;l.data.fight.dragonId=dragon->getUUID();dragon->dragonDeathTime=190;dragon->deathXpReward=12000;for(int t=0;t<10;++t)dragon->tickDeath();assert(dragon->removed&&l.xp==0&&l.eggs==0&&l.data.gateways==0);}
 {Level l;arena(l);auto dragon=make_shared<EnderDragon>(&l);l.addEntity(dragon);EndFight::tick(&l);assert(l.data.fight.previouslyKilled&&!l.data.fight.dragonKilled);assert(EndFight::deathReward(&l,dragon.get())==500);}
}
'''
def main():
 source=(ROOT/'tests/end_fight_fixture.h').read_text()+'\n'+clean('EndFightState.h')+'\n'+(ROOT/'tests/end_fight_fixture_tail.h').read_text()
 for name in ('EndFight.h','EnderCrystal.h','EndCrystalItem.h','SpikeFeature.h'):source+='\n'+clean(name)
 source+='\n'+ENGINE
 for name in ('EndFightState.cpp','EnderCrystal.cpp','EndCrystalItem.cpp','SpikeFeature.cpp','EndFight.cpp'):source+='\n'+clean(name)
 for signature in ('void EnderDragon::tickDeath()','void EnderDragon::spawnExitPortal(','void EnderDragon::addAdditonalSaveData(','void EnderDragon::readAdditionalSaveData('):source+='\n'+method('EnderDragon.cpp',signature)
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'end_fight_production',source+TEST)
 print('Production ritual, save/load, item, crystal, native pillars and dragon death checks: passed')
if __name__=='__main__':main()
