"""Exercise production Mending methods; native CI and in-game acceptance remain separate."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
ROOT=Path(__file__).resolve().parents[1]
W=ROOT/'Minecraft.World'

def method(file,signature):
    s=(W/file).read_text();start=s.index(signature);op=s.index('{',start);depth=1;i=op+1
    while depth:
        depth+=(s[i]=='{')-(s[i]=='}');i+=1
    return s[start:i]

STUB=r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <unordered_map>
using namespace std;
inline const bool DEBUG_COST=false;
struct App { void DebugPrintf(const char*,int,int){} };inline App app;
struct Random { int choice=0; int nextInt(int n){assert(n>0);return choice%n;} float nextFloat(){return .5f;} };
struct Item { inline static const int book_Id=340,enchantedBook_Id=403; };
struct ItemInstance {
    int id=1,count=1,damage=0; bool damageable=true; unordered_map<int,int> ench;
    Item* getItem(){static Item item;return &item;}
    bool isDamageableItem(){return damageable;} bool isDamaged(){return damage>0;}
    int getDamageValue(){return damage;} void setAuxValue(int i){damage=i;}
};
struct EnchantmentCategory {
    inline static const EnchantmentCategory *all=nullptr,*bow=nullptr;
    bool canEnchant(Item*)const{return true;}
};
struct LivingEntity;
struct Enchantment {
    int id,frequency,descriptionId=0; const EnchantmentCategory* category;
    inline static Enchantment *mending=nullptr,*arrowInfinite=nullptr;
    struct Array { Enchantment* data[256]={};unsigned int length=256;Enchantment*& operator[](int i){return data[i];} };
    static Array enchantments;
    Enchantment(int id,int f,const EnchantmentCategory* c):id(id),frequency(f),category(c){enchantments[id]=this;}
    virtual int getMinCost(int){return 1;} virtual int getMaxCost(int){return 100;}
    virtual int getMinLevel(){return 1;} virtual int getMaxLevel(){return 1;}
    virtual bool canEnchant(shared_ptr<ItemInstance>){return true;}
    virtual bool isTreasureOnly()const{return false;}
    virtual bool isCompatibleWith(Enchantment*)const;
    void setDescriptionId(int i){descriptionId=i;}
};
inline Enchantment::Array Enchantment::enchantments;
struct EnchantmentInstance { Enchantment* enchantment;int level; EnchantmentInstance(Enchantment* e,int l):enchantment(e),level(l){} };
struct ItemInstanceArray { vector<shared_ptr<ItemInstance>> data;unsigned int length;ItemInstanceArray(vector<shared_ptr<ItemInstance>> a):data(a),length(a.size()){} shared_ptr<ItemInstance> operator[](int i){return data[i];} };
struct LivingEntity {
    shared_ptr<ItemInstance> hand;vector<shared_ptr<ItemInstance>> armor;
    shared_ptr<ItemInstance> getCarriedItem(){return hand;}
    ItemInstanceArray getEquipmentSlots(){return ItemInstanceArray(armor);}
};
struct Player:LivingEntity {
    int takeXpDelay=0,xp=0,taken=0;struct {bool instabuild=false;} abilities;
    void increaseXp(int v){xp+=v;} void take(shared_ptr<int>,int){++taken;}
};
struct EnchantmentHelper {
    static int getEnchantmentLevel(int id,shared_ptr<ItemInstance> i){return i?i->ench[id]:0;}
    static shared_ptr<ItemInstance> getMendingItem(shared_ptr<LivingEntity>,Random*);
    static unordered_map<int,EnchantmentInstance*>* getAvailableEnchantmentResults(int,shared_ptr<ItemInstance>,bool=false);
};
struct Level { bool isClientSide=false; };
const int IDS_ENCHANTMENT_MENDING=1,IDS_ENCHANTMENT_ARROW_INFINITE=2,eSoundType_RANDOM_ORB=3;
struct ExperienceOrb {
    Level* level;Random* random;int throwTime=0,value=0,sounds=0;bool removed=false;
    void playerTouch(shared_ptr<Player>);
    void playSound(int,float,float){++sounds;} shared_ptr<int> shared_from_this(){return nullptr;}
    void remove(){removed=true;}
};
'''
TEST=r'''
#include "MendingEnchantment.cpp"
#include "ArrowInfiniteEnchantment.cpp"
shared_ptr<ItemInstance> piece(int damage=0,bool mending=true){auto p=make_shared<ItemInstance>();p->damage=damage;if(mending)p->ench[70]=1;return p;}
void pickup(shared_ptr<Player> p,int value,int choice=0){Level l;Random r;r.choice=choice;ExperienceOrb o{&l,&r};o.value=value;o.playerTouch(p);assert(o.removed&&p->taken>0&&p->takeXpDelay==2);p->takeXpDelay=0;}
void pool(bool treasure,int value,shared_ptr<ItemInstance> item,bool expected){
 auto available=EnchantmentHelper::getAvailableEnchantmentResults(value,item,treasure);
 assert((available&&available->count(70))==expected);
 if(available){for(auto& entry:*available)delete entry.second;delete available;}
}
int main(){
 MendingEnchantment m(70,2);ArrowInfiniteEnchantment infinity(51,1);Enchantment ordinary(32,10,nullptr);
 EnchantmentCategory category;ordinary.category=&category;m.category=&category;infinity.category=&category;
 Enchantment::mending=&m;Enchantment::arrowInfinite=&infinity;
 assert(m.descriptionId==IDS_ENCHANTMENT_MENDING&&m.isTreasureOnly()&&m.getMaxLevel()==1);
 assert(m.getMinCost(1)==25&&m.getMaxCost(1)==75);
 assert(!m.isCompatibleWith(&m)&&!m.isCompatibleWith(&infinity)&&!infinity.isCompatibleWith(&m));
 assert(m.isCompatibleWith(&ordinary)&&ordinary.isCompatibleWith(&m));
 auto tool=piece();auto block=piece();block->damageable=false;auto book=piece();book->id=340;book->damageable=false;
 assert(m.canEnchant(tool)&&!m.canEnchant(block)&&!m.canEnchant(nullptr));
 for(int value=1;value<=100;++value){pool(false,value,tool,false);pool(false,value,book,false);pool(true,value,tool,value>=25&&value<=75);pool(true,value,book,value>=25&&value<=75);pool(true,value,block,false);}
 // Held item and all four worn slots qualify, with a uniform choice among five.
 auto p=make_shared<Player>();p->hand=piece(100);for(int i=0;i<4;++i)p->armor.push_back(piece(100));
 Random r;for(int i=0;i<5;++i){r.choice=i;assert(EnchantmentHelper::getMendingItem(p,&r)==(i==0?p->hand:p->armor[i-1]));pickup(p,3,i);}
 assert(p->xp==0&&p->hand->damage==94);for(auto a:p->armor)assert(a->damage==94);
 // Full selection gives XP, without trying another damaged item (old-version behavior).
 p=make_shared<Player>();p->hand=piece();p->armor={piece(20)};pickup(p,4,0);assert(p->xp==4&&p->armor[0]->damage==20);
 pickup(p,4,1);assert(p->xp==4&&p->armor[0]->damage==12);
 // Odd durability uses integer floor for XP consumption; leftover XP is awarded once.
 p=make_shared<Player>();p->hand=piece(3);pickup(p,5);assert(p->hand->damage==0&&p->xp==4);
 p=make_shared<Player>();p->hand=piece(1);pickup(p,1);assert(p->hand->damage==0&&p->xp==1);
 // No Mending, empty stack, invalid non-damageable item and empty equipment are ignored.
 p=make_shared<Player>();p->hand=piece(20,false);pickup(p,7);assert(p->xp==7&&p->hand->damage==20);
 p=make_shared<Player>();p->hand=piece(20);p->hand->count=0;pickup(p,7);assert(p->xp==7&&p->hand->damage==20);
 p=make_shared<Player>();p->hand=block;pickup(p,7);assert(p->xp==7);
 p=make_shared<Player>();pickup(p,7);assert(p->xp==7);
 // A held item included again in an entity equipment array is not weighted twice.
 p=make_shared<Player>();p->hand=piece(10);auto armor=piece(10);p->armor={p->hand,armor};r.choice=1;assert(EnchantmentHelper::getMendingItem(p,&r)==armor);
 // Pickup authority, cooldown and throw delay must precede any mutation.
 for(int gate=0;gate<3;++gate){p=make_shared<Player>();p->hand=piece(20);Level l;Random rng;ExperienceOrb o{&l,&rng};o.value=5;if(gate==0)l.isClientSide=true;if(gate==1)p->takeXpDelay=1;if(gate==2)o.throwTime=1;o.playerTouch(p);assert(!o.removed&&p->hand->damage==20&&p->xp==0&&p->taken==0&&o.sounds==0);}
 p=make_shared<Player>();p->hand=piece(20);pickup(p,0);assert(p->hand->damage==20&&p->xp==0);
}
'''

def main():
    with tempfile.TemporaryDirectory() as tmp:
        f=Path(tmp)
        (f/'stdafx.h').write_text(STUB)
        for name in ['net.minecraft.world.item.h','Enchantment.h']:(f/name).write_text('#include "stdafx.h"\n')
        for name in ['MendingEnchantment.h','MendingEnchantment.cpp','ArrowInfiniteEnchantment.h','ArrowInfiniteEnchantment.cpp']:(f/name).write_bytes((W/name).read_bytes())
        bodies='\n'.join(method(file,sig) for file,sig in [
            ('Enchantment.cpp','bool Enchantment::isCompatibleWith'),
            ('EnchantmentHelper.cpp','shared_ptr<ItemInstance> EnchantmentHelper::getMendingItem'),
            ('EnchantmentHelper.cpp','unordered_map<int, EnchantmentInstance *> *EnchantmentHelper::getAvailableEnchantmentResults'),
            ('ExperienceOrb.cpp','void ExperienceOrb::playerTouch')])
        compile_and_run(f,'mending_production', '#include "stdafx.h"\n'+bodies+'\n'+TEST)
    # Verify the source plumbing used outside the compiled focused fixture.
    h=(W/'EnchantmentHelper.cpp').read_text()
    assert 'selectEnchantment(random, itemInstance, enchantmentCost, allowTreasure)' in h
    assert 'getAvailableEnchantmentResults(realValue, itemInstance, allowTreasure)' in h
    header=(W/'EnchantmentHelper.h').read_text();assert header.count('bool allowTreasure = false')==3
    registration=(W/'Enchantment.cpp').read_text();assert 'new MendingEnchantment(70, FREQ_RARE)' in registration
    assert registration.index('new MendingEnchantment')<registration.index('validEnchantments.push_back')
    assert (W/'FishingHelper.cpp').read_text().count('enchantItem(random, itemInstance, 30, true)')==3
    assert '20+random.nextInt(20),true)' in (W/'EndCityFeature.cpp').read_text()
    villager=(W/'Villager.cpp').read_text();assert 'isTreasureOnly()) cost *= 2' in villager and 'cost = min(cost, 64)' in villager
    assert 'Enchantment::validEnchantments[' in villager
    # Compile the actual anvil applicability and compatibility block in both merge directions.
    source=(W/'RepairMenu.cpp').read_text();start=source.index('bool compatible = enchantment->canEnchant(input);');end=source.index('if (!compatible) continue;',start)
    block=source[start:end]
    with tempfile.TemporaryDirectory() as tmp:
        f=Path(tmp)
        (f/'stdafx.h').write_text(STUB)
        (f/'Enchantment.h').write_text('#include "stdafx.h"\n')
        (f/'net.minecraft.world.item.h').write_text('#include "stdafx.h"\n')
        for name in ['MendingEnchantment.h','MendingEnchantment.cpp','ArrowInfiniteEnchantment.h','ArrowInfiniteEnchantment.cpp']:(f/name).write_bytes((W/name).read_bytes())
        check='''\nbool applicable(Enchantment* enchantment, shared_ptr<ItemInstance> input, unordered_map<int,int>* enchantments,shared_ptr<Player> player) { int id=enchantment->id,price=0,extra=1;\n'''+block+'\nreturn compatible;}\n'
        tests=r'''
int main(){MendingEnchantment m(70,2);ArrowInfiniteEnchantment inf(51,1);Enchantment other(34,5,nullptr);Enchantment::mending=&m;Enchantment::arrowInfinite=&inf;
 auto p=make_shared<Player>();auto input=make_shared<ItemInstance>();unordered_map<int,int> ench;
 assert(applicable(&m,input,&ench,p));input->damageable=false;assert(!applicable(&m,input,&ench,p));input->id=403;assert(applicable(&m,input,&ench,p));
 ench[51]=1;assert(!applicable(&m,input,&ench,p));ench.clear();ench[70]=1;assert(!applicable(&inf,input,&ench,p));ench.clear();ench[34]=3;assert(applicable(&m,input,&ench,p));
}
'''
        compile_and_run(f,'mending_anvil','#include "stdafx.h"\n#include "MendingEnchantment.cpp"\n#include "ArrowInfiniteEnchantment.cpp"\n'+method('Enchantment.cpp','bool Enchantment::isCompatibleWith')+check+tests)
    print('Mending treasure, registration and integration checks: passed')
if __name__=='__main__':main()
