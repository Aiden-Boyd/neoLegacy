#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <memory>
#include <map>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
using namespace std;
struct Random {int choice=0;queue<int> values;int nextInt(int n){assert(n>0);int v=choice;if(!values.empty()){v=values.front();values.pop();}return v%n;}float nextFloat(){return 0.5f;}};
struct Mth {static int floor(double d){return int(std::floor(d));}static int nextInt(Random*r,int lo,int hi){return lo+r->nextInt(hi-lo+1);}};
struct Material {inline static Material *stone=nullptr,*plant=nullptr;};
struct Icon {wstring name;};struct IconRegister {map<wstring,Icon> icons;Icon*registerIcon(const wstring& n){return &(icons[n]=Icon{n});}};
struct AABB {double y0=64;};struct TileEntity {};
struct Item {int id,maxStackSize=64;enum{eBaseItemType_bowl,eMaterial_wood};
 enum{beetroot_Id=434,beetrootSeeds_Id=435,beetrootSoup_Id=436,carrots_Id=391,potato_Id=392,seeds_wheat_Id=295,netherwart_seeds_Id=372,seeds_melon_Id=362,seeds_pumpkin_Id=361,wheat_Id=296};
 inline static Item *bowl=nullptr;Item(int id):id(id+256){}virtual~Item()=default;void setMaxStackSize(int n){maxStackSize=n;}void setBaseItemTypeAndMaterial(int,int){}
};
struct ItemInstance {int id,count,aux;bool frost=false;ItemInstance(int id,int n=1,int a=0):id(id),count(n),aux(a){}ItemInstance(Item*p):id(p->id),count(1),aux(0){}int getAuxValue(){return aux;} shared_ptr<ItemInstance>copy(){return make_shared<ItemInstance>(*this);} };
struct ChatPacket{enum{e_ChatDeathHotFloor,e_ChatDeathHotFloorPlayer};};
struct DamageSource {bool fire=false;static DamageSource*hotFloor;DamageSource(int,int){}DamageSource*setIsFire(){fire=true;return this;}};
struct Enchantment {int id=9;inline static Enchantment *frostWalker=nullptr;};
struct EnchantmentHelper {static int getEnchantmentLevel(int,shared_ptr<ItemInstance>p){return p&&p->frost?1:0;}};
struct Level;struct Entity:enable_shared_from_this<Entity>{bool immune=false,sneaking=false;virtual~Entity()=default;bool isFireImmune(){return immune;}bool isSneaking(){return sneaking;}};
struct LivingEntity:Entity {enum{SLOT_BOOTS=1};double x=.5,z=.5;bool onGround=true,fireResistance=false;float health=20;int hurts=0;AABB box;AABB*bb=&box;Level*level=nullptr;vector<shared_ptr<ItemInstance>>equipment=vector<shared_ptr<ItemInstance>>(5);shared_ptr<ItemInstance>getCarried(int i){return equipment[i];}void hurt(DamageSource*s,float n){if(fireResistance&&s->fire)return;health-=n;++hurts;}void updateHotFloor();};
struct Player:LivingEntity {bool allowed=true;bool mayUseItemAt(int,int,int,int,shared_ptr<ItemInstance>){return allowed;}};
struct Pig{bool isFood(shared_ptr<ItemInstance>);};struct Chicken{bool isFood(shared_ptr<ItemInstance>);};
struct Mob:LivingEntity{bool canSpawn();};
struct LevelSource {map<tuple<int,int,int>,int>blocks,data;int getTile(int x,int y,int z){return blocks[{x,y,z}];}int getData(int x,int y,int z){return data[{x,y,z}];}};
struct Level:LevelSource {enum{MAX_BRIGHTNESS=15};bool isClientSide=false,failWrites=false;int brightness=15,writes=0,particles=0,sounds=0,aliveChecks=0;Random rng;Random*random=&rng;vector<shared_ptr<ItemInstance>>drops;
 bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}int getRawBrightness(int,int,int){return brightness;}
 bool setTileAndData(int x,int y,int z,int id,int a,int){if(failWrites)return false;blocks[{x,y,z}]=id;data[{x,y,z}]=a;++writes;return true;}void setTileAndUpdate(int x,int y,int z,int id){setTileAndData(x,y,z,id,0,3);}void setData(int x,int y,int z,int a,int){if(!failWrites){data[{x,y,z}]=a;++writes;}}
 void playSound(double,double,double,int,float,float){++sounds;}void addParticle(int,double,double,double,double,double,double){++particles;}
 bool isUnobstructed(AABB*){return true;}vector<int>*getCubes(shared_ptr<Entity>,AABB*){static vector<int>empty;return &empty;}bool containsAnyLiquid_NoLoad(AABB*){return false;}
};
struct Facing {enum{DOWN,UP,NORTH,SOUTH,WEST,EAST};};
struct Tile {enum{magma_Id=213,farmland_Id=60,water_Id=8,calmWater_Id=9,SHAPE_TREE=31,SHAPE_ROWS=6,UPDATE_CLIENTS=2,UPDATE_ALL=3,SOUND_GRASS=1};
 inline static Tile*magma=nullptr;int id;Icon*icon=nullptr;float y1=1,emission=0;bool ticking=false;int sentMask=0;
 Tile(int id,Material* =nullptr):id(id){}virtual~Tile()=default;void setShape(float,float,float,float,float yy,float){y1=yy;}void setTicking(bool b){ticking=b;}void setDestroyTime(float){}void setSoundType(int){}void setNotCollectStatistics(){}void sendTileData(int mask=15){sentMask=mask;}void setLightEmission(float f){emission=f;}
 virtual void stepOn(Level*,int,int,int,shared_ptr<Entity>){}virtual void registerIcons(IconRegister*){}virtual int getResource(int,Random*,int){return id;}virtual int getResourceCount(Random*){return 1;}
 virtual int getRenderShape(){return 0;}virtual int getType(int){return 0;}virtual int getSpawnResourcesAuxValue(int){return 0;}
 virtual void spawnResources(Level*l,int,int,int,int data,float,int bonus){if(!l->isClientSide)l->drops.push_back(make_shared<ItemInstance>(getResource(data,l->random,bonus),getResourceCount(l->random),0));}
 void popResource(Level*l,int,int,int,shared_ptr<ItemInstance>i){l->drops.push_back(i);}
};
struct Bush:Tile {Bush(int id):Tile(id){}virtual void tick(Level*l,int x,int y,int z,Random*){++l->aliveChecks;if(l->getTile(x,y-1,z)!=Tile::farmland_Id){spawnResources(l,x,y,z,l->getData(x,y,z),1,0);l->setTileAndUpdate(x,y,z,0);}}};
struct FoodItem:Item {int nutrition;FoodItem(int id,int n,bool):Item(id),nutrition(n){}virtual shared_ptr<ItemInstance>useTimeDepleted(shared_ptr<ItemInstance>i,Level*,shared_ptr<Player>){--i->count;return i;}};
const int eSoundType_RANDOM_FIZZ=1,eParticleType_largesmoke=2;
