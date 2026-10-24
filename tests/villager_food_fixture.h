#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
#define byte unsigned char
struct Tag {enum{TAG_Compound=10,TAG_List=9};virtual ~Tag(){}virtual int getId(){return 0;}};
template<class T>struct ListTag:Tag {vector<T*> data;~ListTag(){for(auto p:data)delete p;}int getId(){return TAG_List;}void add(T*p){data.push_back(p);}int size(){return int(data.size());}T*get(int i){return data[i];}};
struct CompoundTag:Tag {
 map<wstring,int> ints;map<wstring,Tag*> tags;~CompoundTag(){for(auto p:tags)delete p.second;}int getId(){return TAG_Compound;}
 void putByte(wstring n,int v){ints[n]=v;}int getByte(wstring n){return ints[n];}
 void put(wstring n,Tag*t){tags[n]=t;}bool contains(wstring n,int id){return tags.count(n)&&tags[n]->getId()==id;}
 ListTag<Tag>*getList(wstring n){return reinterpret_cast<ListTag<Tag>*>(tags[n]);}
};
struct ItemInstance {
 int id,count,aux=0,tag=0,damage=0;
 ItemInstance(int id,int count=1,int unused=0):id(id),count(count){}
 int getMaxStackSize(){return 64;}shared_ptr<ItemInstance>copy(){return make_shared<ItemInstance>(*this);}
 bool sameItemWithTags(shared_ptr<ItemInstance> p){return id==p->id&&aux==p->aux&&tag==p->tag;}
 void save(CompoundTag*t){t->ints[L"id"]=id;t->ints[L"count"]=count;t->ints[L"aux"]=aux;t->ints[L"tag"]=tag;}
 static shared_ptr<ItemInstance>fromTag(CompoundTag*t){auto p=make_shared<ItemInstance>(t->ints[L"id"],t->ints[L"count"]);p->aux=t->ints[L"aux"];p->tag=t->ints[L"tag"];return p;}
 template<class T>void hurtAndBreak(int n,shared_ptr<T>){damage+=n;}
};
struct Random {mt19937 rng;int forced=-1;float fixed=-1;Random(int64_t seed=0):rng(unsigned(seed^(seed>>32))){}int nextInt(int n){return forced>=0?forced%n:int(rng()%n);}float nextFloat(){return fixed>=0?fixed:float(rng()%100000)/100000;}};
struct Material {bool solid=false;bool isSolid(){return solid;}static Material*dirt;};inline Material dirtMaterial{true};inline Material*Material::dirt=&dirtMaterial;
struct Icon{};struct IconRegister{Icon*registerIcon(const wchar_t*){static Icon i;return &i;}};
struct AABB {double x0,y0,z0,x1,y1,z1;static AABB*newTemp(double x,double y,double z,double xx,double yy,double zz){static AABB a;a={x,y,z,xx,yy,zz};return &a;}AABB*grow(int,int,int){return this;}};
struct Sound {int getStepSound(){return 0;}};
struct Level;struct Player;
struct Tile {
 enum{grass_Id=2,dirt_Id=3,farmland_Id=60,wheat_Id=59,carrots_Id=141,potatoes_Id=142,beetroots_Id=207,grassPath_Id=208,UPDATE_ALL=3,UPDATE_CLIENTS=2};
 inline static Tile*tiles[4096]={};inline static Tile*dirt=nullptr,*grassPath=nullptr;
 Icon*icon=nullptr;Sound*soundType;int id,light=0;bool solid;float shapeY=1;
 Tile(int id,Material* =nullptr,bool solid=true):id(id),solid(solid){tiles[id]=this;static Sound s;soundType=&s;}
 void setLightBlock(int n){light=n;}void setShape(float,float,float,float,float y,float){shapeY=y;}
 virtual Icon*getTexture(int,int){return icon;}bool shouldRenderFace(Level*,int,int,int,int){return true;}
};
struct CropTile:Tile {int age;CropTile(int id,int age):Tile(id),age(age){}int getMaxAge(){return age;}};
using LevelSource=Level;
struct GameRules{enum{RULE_MOBGRIEFING};bool grief=true;bool getBoolean(int){return grief;}};
struct Entity:enable_shared_from_this<Entity> {bool removed=false;virtual~Entity(){}};
struct ItemEntity:Entity {double xd=0,yd=0,zd=0;ItemEntity(){}ItemEntity(Level*,double,double,double,shared_ptr<ItemInstance>i):item(i){}int throwTime=0;shared_ptr<ItemInstance>item;shared_ptr<ItemInstance>getItem(){return item;}void setItem(shared_ptr<ItemInstance>i){item=i;}void remove(){removed=true;}};
using Pos=tuple<int,int,int>;
struct Villager;
struct Village {int doors=10,population=2;bool timer=true;bool isBreedTimerOk(){return timer;}int getDoorCount(){return doors;}int getPopulationSize(){return population;}};
struct Villages {shared_ptr<Village> value=make_shared<Village>();shared_ptr<Village>getClosestVillage(int,int,int,int){return value;}};
struct Level {
 enum{genDepth=256,eSpawnType_Breed};Villages collection;Villages *villages=&collection;bool capacity=true,spawn=true;int births=0,events=0;shared_ptr<Entity> closest;bool canCreateMore(int,int){return capacity;}bool addEntity(shared_ptr<Entity>e){if(!spawn)return false;entities.push_back(e);++births;return true;}void broadcastEntityEvent(shared_ptr<Entity>,int){++events;}shared_ptr<Entity>getClosestEntityOfClass(const type_info&,AABB*,shared_ptr<Entity>){return closest;}bool isClientSide=false,fail=false,loaded=true,protectedCells=false;int writes=0,drops=0,height=65;int64_t seed=42;Random random;GameRules rules;vector<shared_ptr<Entity>>entities;
 map<Pos,int> blocks,data;
 int getTile(int x,int y,int z){auto i=blocks.find({x,y,z});return i==blocks.end()?1:i->second;}
 int getData(int x,int y,int z){return data[{x,y,z}];}Material*getMaterial(int x,int y,int z){static Material solid{true},air{false};return getTile(x,y,z)?&solid:&air;}
 bool setTileAndData(int x,int y,int z,int id,int d,int){if(fail)return false;blocks[{x,y,z}]=id;data[{x,y,z}]=d;++writes;return true;}
 bool destroyTile(int x,int y,int z,bool drop){if(fail)return false;blocks[{x,y,z}]=0;drops+=drop;++writes;return true;}
 bool hasChunkAt(int,int,int){return loaded;}int getHeightmap(int,int){return height;}void*getTileEntity(int,int,int){return protectedCells?this:nullptr;}
 int64_t getSeed(){return seed;}GameRules*getGameRules(){return &rules;}void playSound(double,double,double,int,float,float){}
 vector<shared_ptr<Entity>>*getEntitiesOfClass(const type_info&,AABB*){return new vector<shared_ptr<Entity>>(entities);}
};
struct Player {bool permitted=true;bool mayUseItemAt(int,int,int,int,shared_ptr<ItemInstance>){return permitted;}};
struct DiggerItem {struct Tier{};DiggerItem(int,int,const Tier*,void*){}};struct TileArray{};
struct Control {enum{MoveControlFlag=1,LookControlFlag=2};};
struct Goal {void setRequiredControlFlags(int){};virtual~Goal(){}};
struct Navigation {bool moveTo(shared_ptr<Villager>,double){return true;}bool reachable=true;bool moveTo(double,double,double,double){return reachable;}void stop(){}};
struct LookControl {void setLookAt(shared_ptr<Villager>,int,int){}void setLookAt(double,double,double,float,float){}};
struct Mth {static int floor(double n){return int(std::floor(n));}};
class FarmerInventory;
enum{eTYPE_VILLAGER};struct EntityEvent{enum{LOVE_HEARTS};};
struct Villager:Entity {int age=0,foodShareDelay=0;weak_ptr<Villager>foodShareTarget;bool love=false,visible=true,childAllowed=true;int getAge(){return age;}void setAge(int a){age=a;}bool isInLove(){return love;}void setInLove(bool b){love=b;}void moveTo(double,double,double,int,int){}float getEyeHeight(){return 1.6f;}bool canSee(shared_ptr<Entity>){return visible;}double distanceToSqr(shared_ptr<Villager> v){return (x-v->x)*(x-v->x)+(y-v->y)*(y-v->y)+(z-v->z)*(z-v->z);}shared_ptr<Villager>getBreedOffspring(shared_ptr<Villager>){if(!childAllowed)return nullptr;auto c=make_shared<Villager>();c->level=level;return c;}bool isWilling(bool);void setWilling(bool);void shareFood();enum{PROFESSION_FARMER=0};Level*level;double x=.5,y=65,z=.5;bool baby=false,trading=false,alive=true;int profession=0,pickups=0;FarmerInventory*farmInventory;Navigation nav;LookControl look;Random random;AABB box,*bb=&box;
 bool isBaby(){return baby;}bool isTrading(){return trading;}bool isAlive(){return alive;}int getProfession(){return profession;}FarmerInventory*getFarmInventory(){return farmInventory;}Random*getRandom(){return &random;}
 Navigation*getNavigation(){return &nav;}LookControl*getLookControl(){return &look;}int getMaxHeadXRot(){return 40;}void collectFarmItems();void take(shared_ptr<Entity>,int n){pickups+=n;}
};
