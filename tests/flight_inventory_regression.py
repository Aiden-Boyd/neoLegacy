"""Exercise production movement and Creative pagination against bounded fixtures."""
from pathlib import Path
import importlib.util
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def method(path, signature):
    source = (ROOT / path).read_text(encoding="utf-8")
    start = source.index(signature)
    opening = source.index("{", start)
    end, depth = opening + 1, 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


INVENTORY = r'''
#define _GLIBCXX_ASSERTIONS 1
#include <cassert>
#include <memory>
#include <vector>
using namespace std;
struct Groups:vector<int> {
    using vector<int>::operator=;
    bool operator!=(nullptr_t) const { return !empty(); }
};
struct ItemInstance { int count; ItemInstance(int n=1):count(n){} };
struct Slot {
    shared_ptr<ItemInstance> item;
    void set(shared_ptr<ItemInstance> value) { item=value; }
    void remove(int n) { if(item && (item->count-=n)<=0) item=nullptr; }
};
struct AbstractContainerMenu {
    vector<Slot> slots=vector<Slot>(50);
    Slot* getSlot(int i) { return &slots.at(i); }
};
struct App { bool debug=false; bool DebugArtToolsOn(){return debug;} } app;
struct IUIScene_CreativeMenu {
    inline static vector<shared_ptr<ItemInstance>> categoryGroups[6];
    struct TabSpec {
        static const int MAX_SIZE=50;
        int m_dynamicGroupsCount=0,m_staticGroupsCount=0,m_debugGroupsCount=0;
        Groups m_dynamicGroupsA,m_staticGroupsA,m_debugGroupsA;
        unsigned m_staticPerPage=10;
        void populateMenu(AbstractContainerMenu*,int,unsigned);
    };
};
'''

INVENTORY_TEST = r'''
int main() {
    using Menu=IUIScene_CreativeMenu;
    Menu::TabSpec tab;
    AbstractContainerMenu menu;
    auto& groups=Menu::categoryGroups;
    for(int i=0;i<50;++i) groups[0].push_back(make_shared<ItemInstance>());
    tab.m_dynamicGroupsCount=1;tab.m_dynamicGroupsA={0};
    tab.populateMenu(&menu,0,0);
    assert(menu.slots[0].item && menu.slots[49].item); // exactly 50, no slot 50
    tab.populateMenu(&menu,-1,0);
    for(auto& slot:menu.slots) assert(!slot.item);
    tab.m_dynamicGroupsCount=0;
    groups[0]={make_shared<ItemInstance>(64)};groups[1].clear();
    groups[2]={make_shared<ItemInstance>(32)};
    tab.m_staticGroupsCount=3;tab.m_staticGroupsA={0,1,2};
    tab.populateMenu(&menu,0,0);
    assert(menu.slots[0].item->count==64 && menu.slots[1].item->count==32);
    for(int i=2;i<50;++i) assert(!menu.slots[i].item);
    // A later page must clear entire previous stacks, not just remove one item.
    tab.populateMenu(&menu,0,100);
    for(auto& slot:menu.slots) assert(!slot.item);
    // Empty debug registries and sparse debug groups must never be indexed past end.
    app.debug=true;tab.m_staticGroupsCount=0;tab.m_debugGroupsCount=1;
    tab.m_debugGroupsA={1};tab.populateMenu(&menu,0,0);
    for(auto& slot:menu.slots) assert(!slot.item);
    tab.m_debugGroupsCount=2;tab.m_debugGroupsA={1,2};tab.populateMenu(&menu,0,0);
    assert(menu.slots[0].item->count==32 && !menu.slots[1].item);
}
'''

FLIGHT = r'''
#include <cassert>
#include <cmath>
#include <vector>
constexpr float PI=3.14159265358979323846f;
struct Mth { static double sqrt(double v){return std::sqrt(v);}
    static float cos(float v){return std::cos(v);} static float sin(float v){return std::sin(v);} };
struct Vec3 { double x=0,y=0,z=1; };
struct AABB { double y0=0; static AABB* newTemp(double,double,double,double,double,double){static AABB a;return &a;} };
using AABBList=std::vector<AABB*>;
struct Level { AABBList cubes; AABBList* getCubes(int,AABB*,bool){return &cubes;} };
struct Abilities { bool instabuild=false,flying=false;float getFlyingSpeed(){return .05f;} };
struct LivingEntity { void travel(float,float){} };
struct Player:LivingEntity {
    double x=0,y=100,z=0,xd=1,yd=-.1,zd=0;
    float xRot=0,fallDistance=0,m_elytraImpactYd=0,flyingSpeed=.02f,bbWidth=.6f;
    bool jumping=false,horizontalCollision=false,verticalCollision=false,onGround=false,gliding=true;
    void* riding=nullptr;Abilities abilities;AABB box;AABB* bb=&box;Level world;Level* level=&world;Vec3 look;
    bool isElytraFlying(){return gliding;}
    Vec3* getLookAngle(){return &look;}
    void move(double dx,double dy,double dz){x+=dx;y+=dy;z+=dz;}
    void onElytraKineticDamage(float){}
    void setElytraFlying(bool f){gliding=f;}
    int shared_from_this(){return 0;}
    void checkMovementStatistiscs(double,double,double){}
    void travel(float,float);
};
'''

FLIGHT_TEST = r'''
int main() {
    Player survival,creative,holdingJump;
    creative.abilities.instabuild=holdingJump.abilities.instabuild=true;
    holdingJump.jumping=true;
    for(int tick=0;tick<100;++tick) {
        survival.travel(0,0);creative.travel(0,0);holdingJump.travel(0,0);
        assert(std::abs(survival.yd-creative.yd)<1e-12);
        assert(std::abs(survival.yd-holdingJump.yd)<1e-12);
        assert(std::abs(survival.zd-holdingJump.zd)<1e-12);
    }
    for(float pitch:{-90.f,-45.f,0.f,45.f,90.f}) {
        Player p;p.xRot=pitch;
        p.look.y=-std::sin(pitch*PI/180);p.look.z=std::cos(pitch*PI/180);
        for(int tick=0;tick<100;++tick) p.travel(0,0);
        assert(std::isfinite(p.xd)&&std::isfinite(p.yd)&&std::isfinite(p.zd));
    }
}
'''

FURNACE = r'''
#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>
using namespace std;
struct Item { int id; static const int coal_Id=263; };
struct CoalItem { static const int CHAR_COAL=1; };
struct ItemInstance {
    Item item;int count,aux;
    ItemInstance(int id,int count,int aux):item{id},count(count),aux(aux){}
    Item* getItem(){return &item;}
    int getAuxValue(){return aux;}
    shared_ptr<ItemInstance> copy(){return make_shared<ItemInstance>(*this);}
};
struct Player { void awardStat(int,int){} };
struct GenericStats { static int renewableEnergy(){return 0;} static int param_renewableEnergy(){return 0;} };
struct Slot {
    shared_ptr<ItemInstance> value;
    bool hasItem(){return value!=nullptr;}
    shared_ptr<ItemInstance> getItem(){return value;}
    void set(shared_ptr<ItemInstance> v){value=v;}
    void setChanged(){}
    void onQuickCraft(shared_ptr<ItemInstance>,shared_ptr<ItemInstance>){}
    void onTake(shared_ptr<Player>,shared_ptr<ItemInstance>){}
};
struct FurnaceTileEntity {
    bool wasCharcoalUsed(){return false;}
    static bool isFuel(shared_ptr<ItemInstance> i){return i->getItem()->id==263;}
};
struct FurnaceRecipes {
    unordered_map<int,ItemInstance*> recipies;
    static FurnaceRecipes* getInstance(){static FurnaceRecipes r;return &r;}
    ItemInstance* getResult(int,int=0);
};
struct FurnaceMenu {
    enum {INGREDIENT_SLOT=0,FUEL_SLOT=1,RESULT_SLOT=2,INV_SLOT_START=3,INV_SLOT_END=30,USE_ROW_SLOT_START=30,USE_ROW_SLOT_END=39};
    vector<Slot*> slots;FurnaceTileEntity entity;FurnaceTileEntity* furnace=&entity;
    int destination=-1;bool allowMove=true;
    bool moveItemStackTo(shared_ptr<ItemInstance> s,int start,int,bool){
        destination=start;if(!allowMove)return false;s->count=0;return true;
    }
    shared_ptr<ItemInstance> quickMoveStack(shared_ptr<Player>,int);
};
'''

FURNACE_TEST = r'''
int main() {
    ItemInstance dryResult(19,1,0),fishResult(350,1,1);
    auto recipes=FurnaceRecipes::getInstance();
    recipes->recipies[19|(1<<12)]=&dryResult;
    recipes->recipies[349|(1<<12)]=&fishResult;
    Slot slots[39];FurnaceMenu menu;for(auto& slot:slots)menu.slots.push_back(&slot);
    auto player=make_shared<Player>();
    slots[3].value=make_shared<ItemInstance>(19,64,1);
    assert(menu.quickMoveStack(player,3)!=nullptr && menu.destination==0);
    assert(!slots[3].value);
    slots[3].value=make_shared<ItemInstance>(19,64,0);
    assert(menu.quickMoveStack(player,3)!=nullptr && menu.destination==30);
    slots[3].value=make_shared<ItemInstance>(349,1,1);
    assert(menu.quickMoveStack(player,3)!=nullptr && menu.destination==0);
    slots[3].value=make_shared<ItemInstance>(263,1,0);
    assert(menu.quickMoveStack(player,3)!=nullptr && menu.destination==1);
    menu.allowMove=false;slots[3].value=make_shared<ItemInstance>(19,64,1);
    assert(menu.quickMoveStack(player,3)==nullptr);
    assert(slots[3].value->count==64);
}
'''


def main():
    spec=importlib.util.spec_from_file_location("base_tests",ROOT/"tests/base_stability_regression.py")
    base=importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    with tempfile.TemporaryDirectory() as tmp:
        folder=Path(tmp)
        base.compile_and_run(folder,"creative_pagination",INVENTORY+method(
            "Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp",
            "void IUIScene_CreativeMenu::TabSpec::populateMenu(")+INVENTORY_TEST)
        base.compile_and_run(folder,"creative_glide",FLIGHT+method(
            "Minecraft.World/Player.cpp","void Player::travel(")+FLIGHT_TEST)
        base.compile_and_run(folder,"furnace_transfer",FURNACE+method(
            "Minecraft.World/FurnaceRecipes.cpp","ItemInstance* FurnaceRecipes::getResult(")+method(
            "Minecraft.World/FurnaceMenu.cpp","shared_ptr<ItemInstance> FurnaceMenu::quickMoveStack(")+FURNACE_TEST)


if __name__=="__main__":
    main()
