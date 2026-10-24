#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using EngineByte=unsigned char;
#define byte EngineByte
const double PI=3.14159265358979323846;
enum eINSTANCEOF{eTYPE_ENTITY,eTYPE_FIREWORKS_ROCKET,eTYPE_PLAYER};enum{eSoundType_FIREWORKS_LAUNCH,eParticleType_fireworksspark};struct EntityEvent{enum{FIREWORKS_EXPLODE=17};};struct SharedConstants{enum{TICKS_PER_SECOND=20};};
struct Vec3{double x,y,z;static Vec3*newTemp(double x,double y,double z){static Vec3 p[8];static int i=0;auto*v=&p[(i++)%8];*v={x,y,z};return v;}};
struct AABB{AABB*grow(int,int,int){return this;}};struct Mth{static double sqrt(double n){return std::sqrt(n);}};
struct Tag{enum{TAG_Compound=10,TAG_List=9};virtual~Tag(){}virtual int getId(){return 0;}};
template<class T>struct ListTag:Tag{vector<shared_ptr<T>>list;int getId(){return TAG_List;}int size(){return int(list.size());}T*get(int i){return list[i].get();}void add(T*t){list.emplace_back(t);}};
struct CompoundTag:Tag{
 map<wstring,int>numbers;map<wstring,wstring>strings;map<wstring,shared_ptr<CompoundTag>>compounds;map<wstring,shared_ptr<ListTag<Tag>>>lists;
 int getId(){return TAG_Compound;}void putInt(wstring k,int n){numbers[k]=n;}int getInt(wstring k){return numbers[k];}void putBoolean(wstring k,bool b){numbers[k]=b;}bool getBoolean(wstring k){return numbers[k]!=0;}void putByte(wstring k,int n){numbers[k]=n;}byte getByte(wstring k){return byte(numbers[k]);}
 void putString(wstring k,wstring v){strings[k]=v;}wstring getString(wstring k){return strings[k];}void putCompound(wstring k,CompoundTag*t){compounds[k].reset(t);}CompoundTag*getCompound(wstring k){auto it=compounds.find(k);return it==compounds.end()?nullptr:it->second.get();}ListTag<Tag>*getList(wstring k){auto it=lists.find(k);return it==lists.end()?nullptr:it->second.get();}
 bool contains(wstring k,int type=-1){return(type==-1||type==TAG_Compound)&&compounds.count(k)||(type==-1||type==TAG_List)&&lists.count(k)||type==-1&&(numbers.count(k)||strings.count(k));}
};
struct ItemInstance{int count=1;shared_ptr<CompoundTag>tag;bool hasTag(){return bool(tag);}CompoundTag*getTag(){return tag.get();}shared_ptr<ItemInstance>copy(){auto p=make_shared<ItemInstance>(*this);if(tag)p->tag=make_shared<CompoundTag>(*tag);return p;}void save(CompoundTag*t){t->putInt(L"Count",count);if(tag)t->putCompound(L"Tag",new CompoundTag(*tag));}static shared_ptr<ItemInstance>fromTag(CompoundTag*t){auto p=make_shared<ItemInstance>();p->count=t->getInt(L"Count");if(auto tag=t->getCompound(L"Tag"))p->tag=make_shared<CompoundTag>(*tag);return p;}};
struct SynchedEntityData{map<int,int>numbers;map<int,shared_ptr<ItemInstance>>items;void defineNULL(int n,void*){items[n]=nullptr;}void define(int n,int v){numbers[n]=v;}void set(int n,int v){numbers[n]=v;}void set(int n,shared_ptr<ItemInstance>i){items[n]=i;}int getInteger(int n){return numbers[n];}shared_ptr<ItemInstance>getItemInstance(int n){return items[n];}};
struct Random{int nextInt(int){return 0;}double nextGaussian(){return 0;}};
struct Level;struct DamageSource{inline static DamageSource*fireworks=nullptr;};
struct Entity:enable_shared_from_this<Entity>{Level*level;double x=0,y=0,z=0,xd=0,yd=0,zd=0,xOld=0,yOld=0,zOld=0;float xRot=0,yRot=0,xRotO=0,yRotO=0,heightOffset=0,bbHeight=1.8f;bool removed=false;int entityId;wstring uuid;Random rng,*random=&rng;SynchedEntityData metadata,*entityData=&metadata;AABB box,*bb=&box;
 Entity(Level*l):level(l){static int n=0;entityId=++n;uuid=L"entity-"+to_wstring(entityId);}virtual~Entity(){}virtual eINSTANCEOF GetType(){return eTYPE_ENTITY;}void setSize(float,float h){bbHeight=h;}void setPos(double xx,double yy,double zz){x=xx;y=yy;z=zz;}void move(double xx,double yy,double zz){x+=xx;y+=yy;z+=zz;}virtual void tick(){}void remove(){removed=true;}wstring getUUID(){return uuid;}void handleEntityEvent(byte){}float getBrightness(float){return 1;}int getLightColor(float){return 0;}
};
struct LivingEntity:Entity{float health=100;int hits=0;LivingEntity(Level*l):Entity(l){}virtual void hurt(DamageSource*,float n){health-=n;++hits;}};
struct Player:LivingEntity{bool flying=true;Vec3 look{0,0,1};struct{bool instabuild=false;}abilities;Player(Level*l):LivingEntity(l){}bool isElytraFlying(){return flying;}Vec3*getLookAngle(){return &look;}};
struct HitResult{};
struct Level{bool isClientSide=false,failSpawn=false,wall=false;int launches=0,events=0,particles=0,visualExplosions=0;vector<shared_ptr<Player>>players;vector<shared_ptr<Entity>>entities;
 bool addEntity(shared_ptr<Entity>e){if(failSpawn)return false;entities.push_back(e);return true;}shared_ptr<Entity>getEntity(int id){for(auto p:players)if(p->entityId==id)return p;for(auto e:entities)if(e->entityId==id)return e;return nullptr;}void playEntitySound(shared_ptr<Entity>,int,float,float){++launches;}void addParticle(int,double,double,double,double,double,double){++particles;}void broadcastEntityEvent(shared_ptr<Entity>,int){++events;}
 void createFireworks(double,double,double,double,double,double,CompoundTag*){++visualExplosions;}HitResult*clip(Vec3*,Vec3*,bool,bool){static HitResult h;return wall?&h:nullptr;}vector<shared_ptr<Entity>>*getEntitiesOfClass(const type_info&,AABB*){auto out=new vector<shared_ptr<Entity>>;for(auto p:players)out->push_back(p);for(auto e:entities)if(dynamic_pointer_cast<LivingEntity>(e))out->push_back(e);return out;}
};
struct HtmlString{};struct Item{Item(int){}virtual bool TestUse(shared_ptr<ItemInstance>,Level*,shared_ptr<Player>){return false;}virtual shared_ptr<ItemInstance>use(shared_ptr<ItemInstance>i,Level*,shared_ptr<Player>){return i;}};
