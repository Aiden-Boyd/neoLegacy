#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using EngineByte=unsigned char;
#define byte EngineByte
struct Mth {static int floor(double x){return int(std::floor(x));}};
const double PI=3.14159265358979323846;
enum eINSTANCEOF{eTYPE_ENTITY,eTYPE_ENDER_CRYSTAL,eTYPE_ENDERDRAGON,eTYPE_PLAYER};
enum{eSoundType_MOB_ENDERDRAGON_GROWL,eParticleType_hugeexplosion};
struct App {template<class...T>void DebugPrintf(T...){}};inline App app;
struct Tag {enum{TAG_Byte=1,TAG_Int=3,TAG_String=8,TAG_Compound=10};virtual~Tag(){};};
struct CompoundTag:Tag {
 map<wstring,int> ints;map<wstring,wstring> strings;map<wstring,shared_ptr<CompoundTag>> compounds;
 bool contains(wstring n,int kind=-1){return(kind==-1||kind==TAG_Compound)&&compounds.count(n)||(kind==-1||kind==TAG_Int||kind==TAG_Byte)&&ints.count(n)||(kind==-1||kind==TAG_String)&&strings.count(n);}
 void putInt(wstring n,int v){ints[n]=v;}int getInt(wstring n){return ints[n];}void putShort(wstring n,int v){ints[n]=v;}int getShort(wstring n){return ints[n];}
 void putBoolean(wstring n,bool v){ints[n]=v;}bool getBoolean(wstring n){return ints[n]!=0;}void putString(wstring n,wstring v){strings[n]=v;}wstring getString(wstring n){return strings[n];}
 void putCompound(wstring n,CompoundTag*t){compounds[n].reset(t);}CompoundTag*getCompound(wstring n){return compounds[n].get();}
};
struct Random {int forced=0;Random(int64_t=0){}int nextInt(int n){return forced%n;}float nextFloat(){return .5f;}};
struct AABB {double x0,y0,z0,x1,y1,z1;static AABB*newTemp(double x,double y,double z,double xx,double yy,double zz){static AABB b;b={x,y,z,xx,yy,zz};return &b;}bool intersects(AABB*b){return x1>b->x0&&x0<b->x1&&y1>b->y0&&y0<b->y1&&z1>b->z0&&z0<b->z1;}};
struct SynchedEntityData {map<int,int> values;void define(int id,int v){values[id]=v;}void define(int id,byte v){values[id]=v;}void set(int id,int v){values[id]=v;}void set(int id,byte v){values[id]=v;}int getInteger(int id){return values[id];}byte getByte(int id){return byte(values[id]);}};
struct Level;struct DamageSource;
struct Entity:enable_shared_from_this<Entity> {
 Level*level;double x=0,y=0,z=0,xo=0,yo=0,zo=0;float yRot=0,xRot=0,yBodyRot=0,bbWidth=.6f,bbHeight=1.8f,heightOffset=0;bool blocksBuilding=false,noCulling=false,removed=false,invulnerable=false;int tickCount=0;
 AABB bounds,*bb=&bounds;Random rng,*random=&rng;SynchedEntityData data,*entityData=&data;wstring uuid;
 Entity(Level*l):level(l){static int next=0;uuid=L"entity-"+to_wstring(++next);setPos(0,0,0);}virtual~Entity(){}virtual eINSTANCEOF GetType(){return eTYPE_ENTITY;}bool instanceof(eINSTANCEOF t){return GetType()==t;}
 void setSize(float w,float h){bbWidth=w;bbHeight=h;setPos(x,y,z);}void setPos(double xx,double yy,double zz){x=xx;y=yy;z=zz;bounds={x-bbWidth/2,y-heightOffset,z-bbWidth/2,x+bbWidth/2,y-heightOffset+bbHeight,z+bbWidth/2};}
 void moveTo(double x,double y,double z,float yr,float xr){setPos(x,y,z);yRot=yr;xRot=xr;}void move(double dx,double dy,double dz){setPos(x+dx,y+dy,z+dz);}
 void remove(){removed=true;}wstring getUUID(){return uuid;}void setUUID(wstring id){uuid=id;}
 bool isInvulnerable(){return invulnerable;}void setInvulnerable(bool v){invulnerable=v;}virtual bool shouldRenderAtSqrDistance(double d){return d<4096;}
 virtual void addAdditonalSaveData(CompoundTag*){}virtual void readAdditionalSaveData(CompoundTag*){}
 CompoundTag save(){CompoundTag t;t.putString(L"UUID",uuid);t.putBoolean(L"Invulnerable",invulnerable);addAdditonalSaveData(&t);return t;}
 void load(CompoundTag*t){uuid=t->getString(L"UUID");invulnerable=t->getBoolean(L"Invulnerable");readAdditionalSaveData(t);}
 virtual bool hurt(DamageSource*,float){return true;}
};
struct Abilities {bool instabuild=false;};
struct Player:Entity {bool allowed=true;Abilities abilities;Player(Level*l):Entity(l){}bool mayUseItemAt(int,int,int,int,shared_ptr<struct ItemInstance>){return allowed;}};
struct Material {bool replaceable=false;bool isReplaceable(){return replaceable;}};
struct Tile {enum {obsidian_Id=49,unbreakable_Id=7,fire_Id=51,endPortalTile_Id=119,endStone_Id=121,ironFence_Id=101,dragonEgg_Id=122,torch_Id=50,UPDATE_ALL=3,UPDATE_CLIENTS=2};};
struct Dimension {int id=1;};
struct GameRules {enum{RULE_DOMOBLOOT};bool loot=true;bool getBoolean(int){return loot;}};
struct ItemInstance {int count=1;};
struct Item {Item(int){}virtual~Item(){};};
// EndFightState is supplied by the production header before the fixture tail.
