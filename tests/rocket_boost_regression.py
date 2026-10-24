from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from flight_inventory_regression import method
ROOT=Path(__file__).resolve().parents[1]
def clean(path):return '\n'.join(l for l in (ROOT/path).read_text().splitlines()if not l.startswith('#include')and not l.startswith('#pragma'))
TEST=r'''
shared_ptr<ItemInstance> rocketItem(int flight,int stars=0){auto i=make_shared<ItemInstance>();i->count=8;i->tag=make_shared<CompoundTag>();auto f=new CompoundTag();f->putByte(FireworksItem::TAG_FLIGHT,flight);if(stars){auto list=make_shared<ListTag<Tag>>();for(int s=0;s<stars;++s)list->add(new CompoundTag());f->lists[FireworksItem::TAG_EXPLOSIONS]=list;}i->tag->putCompound(FireworksItem::TAG_FIREWORKS,f);return i;}
int main(){DamageSource fireworkDamage;DamageSource::fireworks=&fireworkDamage;FireworksItem item(145);
 for(int flight=1;flight<=3;++flight){Level l;auto p=make_shared<Player>(&l);p->setPos(1,64,2);l.players.push_back(p);auto source=rocketItem(flight);assert(item.TestUse(source,&l,p));item.use(source,&l,p);assert(source->count==7&&l.entities.size()==1);auto rocket=dynamic_pointer_cast<FireworksRocketEntity>(l.entities[0]);assert(rocket);assert(rocket->metadata.getItemInstance(8)!=source&&rocket->metadata.getItemInstance(8)->count==1);rocket->tick();assert(std::abs(p->zd-.85)<1e-9&&p->xd==0&&p->yd==0);assert(rocket->x==p->x&&rocket->z==p->z&&l.launches==1);for(int n=1;n<10*(flight+1);++n)rocket->tick();assert(!rocket->removed);rocket->tick();assert(rocket->removed&&l.events==1&&p->hits==0);}
 {Level l;auto p=make_shared<Player>(&l);l.players.push_back(p);auto source=rocketItem(1);p->flying=false;assert(!item.TestUse(source,&l,p));item.use(source,&l,p);assert(l.entities.empty()&&source->count==8);p->flying=true;l.failSpawn=true;item.use(source,&l,p);assert(source->count==8);l.failSpawn=false;assert(item.useOn(source,p,&l,0,64,0,1,0,0,0,true));assert(source->count==8&&l.entities.empty());assert(item.useOn(source,p,&l,0,64,0,1,0,0,0,false));assert(source->count==7);auto ground=dynamic_pointer_cast<FireworksRocketEntity>(l.entities.back());ground->tick();assert(p->zd==0&&ground->yd>.05);}
 {Level l;l.isClientSide=true;auto p=make_shared<Player>(&l);auto source=rocketItem(1);item.use(source,&l,p);assert(source->count==7&&l.entities.empty());}
 {Level l;auto p=make_shared<Player>(&l);l.players.push_back(p);auto source=rocketItem(3);auto rocket=make_shared<FireworksRocketEntity>(&l,source,p);rocket->tick();CompoundTag save;rocket->addAdditonalSaveData(&save);auto loaded=make_shared<FireworksRocketEntity>(&l);loaded->readAdditionalSaveData(&save);double before=p->zd;loaded->tick();assert(p->zd>before&&loaded->metadata.getInteger(9)==p->entityId);p->flying=false;before=p->zd;loaded->tick();assert(p->zd==before&&loaded->x==p->x);l.players.clear();p->removed=true;for(int n=0;n<22;++n)loaded->tick();assert(loaded->removed);}
 {Level l;l.isClientSide=true;auto p=make_shared<Player>(&l);l.players.push_back(p);auto rocket=make_shared<FireworksRocketEntity>(&l);rocket->metadata.set(9,p->entityId);rocket->tick();assert(p->zd>.8&&l.particles==1);rocket->lerpMotion(.2,.3,.4);assert(rocket->xd==.2&&rocket->yd==.3&&rocket->zd==.4);}
 for(bool wall:{false,true}){Level l;l.wall=wall;auto p=make_shared<Player>(&l);auto near=make_shared<LivingEntity>(&l);near->setPos(2,0,0);l.players.push_back(p);l.entities.push_back(near);auto source=rocketItem(1,2);auto rocket=make_shared<FireworksRocketEntity>(&l,source,p);for(int n=0;n<21;++n)rocket->tick();assert(rocket->removed&&p->hits==1&&p->health==91);assert(near->hits==(wall?0:1));if(!wall)assert(std::abs(near->health-(100-9*std::sqrt(.6)))<.001);}
 {Level l;auto p=make_shared<Player>(&l);auto source=rocketItem(1,1);auto rocket=make_shared<FireworksRocketEntity>(&l,source,p);CompoundTag save;rocket->addAdditonalSaveData(&save);save.putString(L"BoostedUUID",L"missing-owner");auto loaded=make_shared<FireworksRocketEntity>(&l);loaded->readAdditionalSaveData(&save);for(int n=0;n<21;++n)loaded->tick();assert(loaded->removed&&p->hits==0);}
}
'''
def main():
 source=(ROOT/'tests/rocket_boost_fixture.h').read_text()+'\n'+clean('Minecraft.World/FireworksItem.h')+'\n'+clean('Minecraft.World/FireworksRocketEntity.h')+'\n'
 source+='\n'.join(l for l in (ROOT/'Minecraft.World/FireworksItem.cpp').read_text().splitlines()if l.startswith('const wstring FireworksItem::'))+'\nFireworksItem::FireworksItem(int id):Item(id){}\n'
 for signature in ['bool FireworksItem::TestUse(','shared_ptr<ItemInstance> FireworksItem::use(','bool FireworksItem::useOn(']:source+=method('Minecraft.World/FireworksItem.cpp',signature)+'\n'
 source+=clean('Minecraft.World/FireworksRocketEntity.cpp')
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'rocket_boost_and_damage',source+TEST)
 print('Rocket attachment, Flight 1-3 lifetime, client boost, save/reload, missing owners, stars and line-of-sight damage: passed')
if __name__=='__main__':main()
