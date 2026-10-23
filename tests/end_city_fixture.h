#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
using namespace std;
using fixture_byte=unsigned char;
#define byte fixture_byte
enum eINSTANCEOF {eTYPE_SHULKER,eTYPE_SHULKER_BULLET,eTYPE_ARROW,eTYPE_PLAYER,eTYPE_ITEM_FRAME,eTYPE_OTHER};
enum {eParticleType_endRod,eSoundType_SHULKER_AMBIENT,eSoundType_SHULKER_CLOSE,eSoundType_SHULKER_DEATH,eSoundType_SHULKER_HURT,eSoundType_SHULKER_HURT_CLOSED,eSoundType_SHULKER_OPEN,eSoundType_SHULKER_SHOOT,eSoundType_SHULKER_BULLET_HIT,eSoundType_MOB_ENDERMEN_PORTAL};
struct Random {uint64_t state;explicit Random(int64_t seed=0):state(seed){}int nextInt(int n){state=state*6364136223846793005ULL+1442695040888963407ULL;return int((state>>32)%n);}float nextFloat(){return nextInt(16777216)/16777216.f;}};
struct intArray {int *data=nullptr;unsigned length=0;explicit intArray(unsigned n=0):data(n?new int[n]:nullptr),length(n){}int &operator[](unsigned i){return data[i];}};
struct byteArray {byte *data;unsigned length;explicit byteArray(unsigned n):data(new byte[n]()),length(n){}byte &operator[](unsigned i){return data[i];}};
struct BiomeArray {};
struct CompoundTag {
 map<wstring,int64_t> ints;map<wstring,double> doubles;map<wstring,wstring> strings;map<wstring,vector<int>> arrays;
 void putByte(wstring k,byte v){ints[k]=v;}byte getByte(wstring k){return byte(ints[k]);}void putInt(wstring k,int v){ints[k]=v;}int getInt(wstring k){return int(ints[k]);}
 void putString(wstring k,wstring v){strings[k]=v;}wstring getString(wstring k){return strings[k];}void putDouble(wstring k,double v){doubles[k]=v;}double getDouble(wstring k){return doubles[k];}
 bool contains(wstring k){return arrays.count(k)||ints.count(k)||strings.count(k);}void putIntArray(wstring k,intArray a){arrays[k]=vector<int>(a.data,a.data+a.length);delete[] a.data;}intArray getIntArray(wstring k){intArray a;a.data=arrays[k].data();a.length=unsigned(arrays[k].size());return a;}
};
struct AABB {
 double x0,y0,z0,x1,y1,z1;
 AABB *set(double a,double b,double c,double d,double e,double f){x0=a;y0=b;z0=c;x1=d;y1=e;z1=f;return this;}
 static AABB *newTemp(double a,double b,double c,double d,double e,double f){static AABB pool[4096];static unsigned i=0;return pool[(i++)%4096].set(a,b,c,d,e,f);}
 AABB *expand(double a,double b,double c){return newTemp(x0+(a<0?a:0),y0+(b<0?b:0),z0+(c<0?c:0),x1+(a>0?a:0),y1+(b>0?b:0),z1+(c>0?c:0));}
 AABB *grow(double a,double b,double c){return newTemp(x0-a,y0-b,z0-c,x1+a,y1+b,z1+c);}
};
struct Level;struct LivingEntity;struct DamageSource;struct ItemInstance;
struct Item {int id;explicit Item(int id):id(id){}static constexpr int shulkerShell_Id=450;static Item *elytra,*potion;};
inline Item elytraItem(443),potionItem(373);inline Item *Item::elytra=&elytraItem,*Item::potion=&potionItem;
struct ItemInstance {int id,count,aux;bool enchanted=false;ItemInstance(int id,int n=1,int a=0):id(id),count(n),aux(a){}ItemInstance(Item*i,int n=1,int a=0):ItemInstance(i->id,n,a){}};
struct SynchedEntityData {map<int,int> values;void define(int k,byte v){values[k]=v;}byte getByte(int k){return byte(values[k]);}void set(int k,byte v){values[k]=v;}};
struct Entity:enable_shared_from_this<Entity> {
 Level *level;double x=0,y=0,z=0,xd=0,yd=0,zd=0;float bbWidth=1,bbHeight=1,heightOffset=0,fallDistance=0,yRot=0,xRot=0;int tickCount=0;bool removed=false;shared_ptr<Entity> riding;AABB storage{},*bb=&storage;wstring uuid=L"entity";Random rng{42},*random=&rng;
 shared_ptr<SynchedEntityData> entityData=make_shared<SynchedEntityData>();
 explicit Entity(Level*l):level(l){setPos(0,0,0);}virtual ~Entity()=default;virtual eINSTANCEOF GetType(){return eTYPE_OTHER;}bool instanceof(eINSTANCEOF t){return GetType()==t;}
 void setSize(float w,float h){bbWidth=w;bbHeight=h;setPos(x,y,z);}void setPos(double a,double b,double c){x=a;y=b;z=c;bb->set(x-bbWidth/2,y-heightOffset,z-bbWidth/2,x+bbWidth/2,y-heightOffset+bbHeight,z+bbWidth/2);}
 void playSound(int,float,float){}
 void moveTo(double a,double b,double c,float r,float p){setPos(a,b,c);yRot=r;xRot=p;}void move(double a,double b,double c){setPos(x+a,y+b,z+c);}virtual void tick(){++tickCount;}
 virtual AABB *getCollideBox(){return nullptr;}virtual bool isPushable(){return true;}virtual bool isPickable(){return true;}virtual bool hurt(DamageSource*,float){return true;}virtual bool isAlive(){return !removed;}void remove(){removed=true;}
 double distanceToSqr(shared_ptr<Entity> e){return (x-e->x)*(x-e->x)+(y-e->y)*(y-e->y)+(z-e->z)*(z-e->z);}wstring getUUID(){return uuid;}
 virtual void addAdditonalSaveData(CompoundTag*){}virtual void readAdditionalSaveData(CompoundTag*){}
};
struct DamageSource {shared_ptr<Entity> direct;shared_ptr<Entity> getDirectEntity(){return direct;}static DamageSource *thrown(shared_ptr<Entity> e,shared_ptr<Entity>){return new DamageSource{e};}};
struct MobEffect {int id;static MobEffect *levitation;};inline MobEffect levitationEffect{25};inline MobEffect *MobEffect::levitation=&levitationEffect;
struct MobEffectInstance {int id,duration,amplifier;MobEffectInstance(int i,int d,int a):id(i),duration(d),amplifier(a){}int getAmplifier(){return amplifier;}};
struct AttributeInstance {double value=0;void setBaseValue(double v){value=v;}};
struct SharedMonsterAttributes {enum {MAX_HEALTH,MOVEMENT_SPEED,FOLLOW_RANGE};};
struct LivingEntity:Entity {
 AttributeInstance attributes[3];float health=20;int effectId=-1,effectDuration=0,effectAmplifier=0;bool sight=true;
 using Entity::Entity;virtual void defineSynchedData(){}virtual void registerAttributes(){}AttributeInstance *getAttribute(int i){return &attributes[i];}
 float getMaxHealth(){return float(attributes[0].value);}float getHealth(){return health;}void setHealth(float h){health=h;}bool isAlive() override{return health>0&&!removed;}virtual int getArmorValue(){return 0;}
 bool hurt(DamageSource*,float amount) override{health-=amount*(1-getArmorValue()*.04f);return true;}void addEffect(MobEffectInstance *e){effectId=e->id;effectDuration=e->duration;effectAmplifier=e->amplifier;delete e;}
 virtual void aiStep(){}virtual void travel(float,float){}bool canSee(shared_ptr<LivingEntity>){return sight;}
};
struct Player:LivingEntity {bool creative=false;eINSTANCEOF GetType() override{return eTYPE_PLAYER;}using LivingEntity::LivingEntity;};
struct Monster:LivingEntity {
 int xpReward=0,drops=0;shared_ptr<LivingEntity> selected;
 using LivingEntity::LivingEntity;
 virtual void serverAiStep(){}virtual bool useNewAi(){return false;}virtual bool removeWhenFarAway(){return true;}virtual int getAmbientSound(){return -1;}virtual int getHurtSound(){return -1;}virtual int getDeathSound(){return -1;}virtual void dropDeathLoot(bool,int){}
 void setPersistenceRequired(){}shared_ptr<LivingEntity> getTarget(){return selected;}void setTarget(shared_ptr<LivingEntity> p){selected=p;}void lookAt(shared_ptr<Entity>,float,float){}
 void aiStep() override{travel(0,0);}void tick() override{Entity::tick();aiStep();}void spawnAtLocation(int,int){++drops;}
};
struct HitResult {enum {TILE,ENTITY};int type;shared_ptr<Entity> entity;};
struct Throwable:Entity {
 shared_ptr<LivingEntity> owner;bool inGround=false;
 explicit Throwable(Level*l):Entity(l){}Throwable(Level*l,shared_ptr<LivingEntity> e):Entity(l),owner(e){}
 virtual float getGravity(){return .03f;}virtual void onHit(HitResult*)=0;shared_ptr<LivingEntity> getOwner(){return owner;}
 void tick() override{Entity::tick();move(xd,yd,zd);xd*=.99;yd=yd*.99-getGravity();zd*=.99;}
};
struct TileEntity {int changes=0;virtual ~TileEntity()=default;void setChanged(){++changes;}};
struct ChestTileEntity:TileEntity {vector<shared_ptr<ItemInstance>> items=vector<shared_ptr<ItemInstance>>(27);unsigned getContainerSize(){return unsigned(items.size());}shared_ptr<ItemInstance> getItem(unsigned i){return items.at(i);}void setItem(unsigned i,shared_ptr<ItemInstance> v){items.at(i)=v;}};
struct SkullTileEntity:TileEntity {int type=0,rotation=0;void setSkullType(int t,wstring){type=t;}void setRotation(int r){rotation=r;}};
struct BrewingStandTileEntity:TileEntity {map<int,shared_ptr<ItemInstance>> items;void setItem(int i,shared_ptr<ItemInstance> v){items[i]=v;}};
struct Material {bool blocksMotion(){return true;}};
struct Tile {static constexpr int UPDATE_CLIENTS=2;inline static Tile *tiles[256]={};int id;explicit Tile(int i):id(i){tiles[i]=this;}bool isCubeShaped(){return id!=198&&id!=177&&id!=54&&id!=144;}Material mat,*material=&mat;};
struct LevelData {
 set<int> endCityChunks;virtual ~LevelData()=default;virtual const set<int> &getEndCityChunks();virtual bool hasEndCityChunk(int,int);virtual void markEndCityChunk(int,int);
 void saveCities(CompoundTag*);void loadCities(CompoundTag*);void copyCities(LevelData *other);
};
struct DerivedLevelData:LevelData {LevelData *wrapped;explicit DerivedLevelData(LevelData *l):wrapped(l){}const set<int> &getEndCityChunks() override;bool hasEndCityChunk(int,int) override;void markEndCityChunk(int,int) override;};
struct Dimension {int id=1,width=256;int getXZSize(){return width;}};
struct Level {
 static constexpr int genDepth=128,genDepthBits=7,genDepthBitsPlusFour=11,maxBuildHeight=256;
 bool isClientSide=false,loaded=true,liquid=false;int difficulty=2,particles=0;int64_t seed=42;Dimension dim,*dimension=&dim;LevelData meta;
 map<tuple<int,int,int>,int> blocks,data;map<tuple<int,int,int>,shared_ptr<TileEntity>> tileEntities;vector<shared_ptr<Entity>> entities,nearby;vector<AABB*> collisions;shared_ptr<Player> nearest;
 int getTile(int x,int y,int z){auto i=blocks.find({x,y,z});return i==blocks.end()?0:i->second;}bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}bool hasChunksAt(int,int,int,int,int,int){return loaded;}
 vector<AABB*> *getCubes(shared_ptr<Entity>,AABB *box){collisions.clear();for(int x=int(floor(box->x0));x<int(ceil(box->x1));++x)for(int y=int(floor(box->y0));y<int(ceil(box->y1));++y)for(int z=int(floor(box->z0));z<int(ceil(box->z1));++z){int id=getTile(x,y,z);if(id&&id!=198&&id!=177)collisions.push_back(box);}return &collisions;}
 bool containsAnyLiquid(AABB*){return liquid;}vector<shared_ptr<Entity>> *getEntities(shared_ptr<Entity> except,AABB *box){nearby.clear();for(auto e:entities)if(e!=except&&e->x>=box->x0&&e->x<=box->x1&&e->y>=box->y0&&e->y<=box->y1&&e->z>=box->z0&&e->z<=box->z1)nearby.push_back(e);return &nearby;}
 shared_ptr<Player> getNearestAttackablePlayer(shared_ptr<Entity>,int){return nearest&&!nearest->creative?nearest:nullptr;}bool addEntity(shared_ptr<Entity> e){entities.push_back(e);return true;}void addParticle(int,double,double,double,double,double,double){++particles;}
 int64_t getSeed(){return seed;}LevelData *getLevelData(){return &meta;}shared_ptr<TileEntity> getTileEntity(int x,int y,int z){auto i=tileEntities.find({x,y,z});return i==tileEntities.end()?nullptr:i->second;}
 void setTileAndData(int x,int y,int z,int id,int d,int){auto key=make_tuple(x,y,z);int old=getTile(x,y,z);blocks[key]=id;data[key]=d;if(old!=id){tileEntities.erase(key);if(id==54)tileEntities[key]=make_shared<ChestTileEntity>();if(id==144)tileEntities[key]=make_shared<SkullTileEntity>();if(id==117)tileEntities[key]=make_shared<BrewingStandTileEntity>();}}
};
struct Difficulty {static constexpr int PEACEFUL=0;};
struct ItemFrame:Entity {shared_ptr<ItemInstance> item;int direction;ItemFrame(Level*l,int x,int y,int z,int d):Entity(l),direction(d){double dx[4]={0,-.5625,0,.5625},dz[4]={.5625,0,-.5625,0};setPos(x+.5+dx[d],y+.5,z+.5+dz[d]);}eINSTANCEOF GetType() override{return eTYPE_ITEM_FRAME;}void setItem(shared_ptr<ItemInstance> i,bool){item=i;}};
struct Direction {static constexpr int SOUTH=0;};
struct EnchantmentHelper {static shared_ptr<ItemInstance> enchantItem(Random*,shared_ptr<ItemInstance> i,int level){assert(level>=20&&level<=39);i->enchanted=true;return i;}};
struct EndIslandTerrain {static bool isOuterChunk(int x,int z){return int64_t(x)*x+int64_t(z)*z>4096;}};
struct TheEndLevelRandomLevelSource {void prepareHeights(int,int,byteArray b,BiomeArray){for(int x=0;x<16;++x)for(int z=0;z<16;++z)for(int y=0;y<=70;++y)b[(x<<11)|(z<<7)|y]=121;}};
