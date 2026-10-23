#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;
#define byte unsigned char
using eINSTANCEOF=int;
enum {eTYPE_SHULKERBOXENTITY=20,IDS_TILE_SHULKER_BOX=900,IDS_SHULKER_BOX_MORE=901,eSoundType_SHULKER_BOX_OPEN=1,eSoundType_SHULKER_BOX_CLOSE=2};
struct App {const wchar_t*GetString(int id){return id==IDS_SHULKER_BOX_MORE?L"... and %d more":L"Shulker Box";}};inline App app;
#define swprintf_s swprintf
struct Tag {enum{TAG_String=8,TAG_List=9,TAG_Compound=10};virtual ~Tag()=default;virtual Tag *copy()=0;virtual int getId(){return TAG_Compound;}};
template<class T>struct ListTag:Tag {vector<T*> values;~ListTag(){for(auto p:values)delete p;}int getId()override{return TAG_List;}void add(T*p){values.push_back(p);}int size(){return int(values.size());}T*get(int i){return values.at(i);}Tag*copy()override{auto r=new ListTag;for(auto p:values)r->add(static_cast<T*>(p->copy()));return r;}};
struct CompoundTag:Tag {
 map<wstring,int> numbers;map<wstring,wstring> strings;map<wstring,unique_ptr<Tag>> children;
 Tag*copy()override{auto r=new CompoundTag;r->numbers=numbers;r->strings=strings;for(auto &p:children)r->children[p.first].reset(p.second->copy());return r;}
 bool contains(wstring k){return numbers.count(k)||strings.count(k)||children.count(k);}
 bool contains(wstring k,int type){return type==TAG_String?strings.count(k):children.count(k)&&children[k]->getId()==type;}
 void putByte(wstring k,byte v){numbers[k]=v;}void putInt(wstring k,int v){numbers[k]=v;}void putString(wstring k,wstring v){strings[k]=v;}
 int getByte(wstring k){return numbers[k];}int getInt(wstring k){return numbers[k];}wstring getString(wstring k){return strings[k];}
 void put(wstring k,Tag*v){children[k].reset(v);}CompoundTag*getCompound(wstring k){if(!children.count(k))put(k,new CompoundTag);return dynamic_cast<CompoundTag*>(children[k].get());}
 ListTag<CompoundTag>*getList(wstring k){if(!children.count(k))put(k,new ListTag<CompoundTag>);return static_cast<ListTag<CompoundTag>*>(children[k].get());}
};
struct Item {enum{dye_powder_Id=351};int id=0;bool hasCraftingRemainingItem(){return false;}Item*getCraftingRemainingItem(){return this;}};
struct ItemInstance {
 int id,count,aux;unique_ptr<CompoundTag>tag;wstring name;
 ItemInstance(int id,int count,int aux):id(id),count(count),aux(aux){}
 explicit ItemInstance(Item*item):ItemInstance(item->id,1,0){}
 int getAuxValue()const{return aux;}void setAuxValue(int v){aux=v;}Item*getItem(){static Item item;item.id=id;return &item;}
 int getMaxStackSize(){return id>=219&&id<=234?1:64;}bool hasTag(){return bool(tag);}CompoundTag*getTag(){return tag.get();}void setTag(CompoundTag*t){tag.reset(t);}
 bool hasCustomHoverName(){return !name.empty();}wstring getHoverName(){return name;}void setHoverName(wstring n){name=n;}
 CompoundTag*save(CompoundTag*t){t->putInt(L"id",id);t->putInt(L"Count",count);t->putInt(L"Damage",aux);if(tag)t->put(L"tag",tag->copy());if(!name.empty())t->putString(L"name",name);return t;}
 static shared_ptr<ItemInstance>fromTag(CompoundTag*t){auto r=make_shared<ItemInstance>(t->getInt(L"id"),t->getInt(L"Count"),t->getInt(L"Damage"));if(t->contains(L"tag"))r->setTag(static_cast<CompoundTag*>(t->getCompound(L"tag")->copy()));r->name=t->getString(L"name");return r;}
 static shared_ptr<ItemInstance>clone(shared_ptr<ItemInstance>p){if(!p)return nullptr;CompoundTag t;p->save(&t);return fromTag(&t);}
 shared_ptr<ItemInstance>copy(){return clone(shared_ptr<ItemInstance>(this,[](ItemInstance*){}));}
 shared_ptr<ItemInstance>remove(int n){auto result=copy();result->count=n;count-=n;return result;}
};
struct AABB {double x0,y0,z0,x1,y1,z1;static AABB*newTemp(double a,double b,double c,double d,double e,double f){static AABB v;v={a,b,c,d,e,f};return &v;}};
struct Entity {double xd=0,yd=0,zd=0;virtual ~Entity()=default;bool isPushable(){return true;}void move(double x,double y,double z){xd+=x;yd+=y;zd+=z;}};
struct LivingEntity:Entity{};
struct Container;struct Player:LivingEntity {struct {bool instabuild=false;}abilities;int opened=0;void openContainer(shared_ptr<Container>){++opened;}};
struct Facing {enum{DOWN=0,UP=1};inline static int STEP_X[6]={0,0,0,0,-1,1},STEP_Y[6]={-1,1,0,0,0,0},STEP_Z[6]={0,0,-1,1,0,0};};
struct TileEntity;struct Tile;
struct Level {
 static const int maxBuildHeight=256;
 bool allowPlacement=true,allowSet=true,supported=false;int placeCalls=0,placedX=0,placedY=0,placedZ=0;
 bool isClientSide=false,blocked=false;int data=1;shared_ptr<TileEntity>te;Tile*worldTile=nullptr;
 vector<shared_ptr<ItemInstance>> drops;vector<shared_ptr<Entity>>entities;vector<AABB*>cubes;int sounds=0,events=0;
 shared_ptr<TileEntity>getTileEntity(int,int,int){return te;}void removeTileEntity(int,int,int){te.reset();}int getData(int,int,int){return data;}
 vector<AABB*>*getCubes(shared_ptr<Entity>,AABB*b){cubes.clear();if(blocked)cubes.push_back(b);return &cubes;}
 vector<shared_ptr<Entity>>*getEntities(shared_ptr<Entity>,AABB*){return &entities;}
 void tileEvent(int,int,int,int,int,int){++events;}void playSound(double,double,double,int,float,float){++sounds;}
 void updateNeighbourForOutputSignal(int,int,int,int){}
 bool isEmptyTile(int,int,int){return !supported;}
 bool mayPlace(int,int,int,int,bool,int,shared_ptr<Entity>,shared_ptr<ItemInstance>){++placeCalls;return allowPlacement;}
 bool setTileAndData(int,int,int,int,int,int);
 void removeTile(int,int,int){te.reset();worldTile=nullptr;}
};
struct Container {virtual ~Container()=default;virtual int getContainerType(){return 0;}virtual shared_ptr<ItemInstance>getItem(unsigned){return nullptr;}virtual void setItem(unsigned,shared_ptr<ItemInstance>){}virtual int getMaxStackSize()const{return 64;}};
struct Material {enum {PUSH_DESTROY=1};inline static Material *stone=nullptr;};
struct Random{};struct Icon{};struct IconRegister{Icon*registerIcon(wstring){return nullptr;}};
struct TileEntity:enable_shared_from_this<TileEntity> {
 Level*level=nullptr;int x=0,y=0,z=0,data=1;bool removed=false;
 virtual ~TileEntity()=default;virtual eINSTANCEOF GetType(){return 0;}virtual void load(CompoundTag*){}virtual void save(CompoundTag*){}
 virtual void tick(){}virtual bool triggerEvent(int,int){return false;}virtual void clearCache(){}virtual void setRemoved(){removed=true;}bool isRemoved(){return removed;}
 virtual shared_ptr<TileEntity>clone()=0;void clone(shared_ptr<TileEntity>p){p->level=level;p->x=x;p->y=y;p->z=z;p->data=data;}
 int getData(){return level?level->data:data;}Tile*getTile(){return level->worldTile;}bool hasLevel(){return level!=nullptr;}void setChanged(){}
};
struct ChestTileEntity:TileEntity,Container {
 array<shared_ptr<ItemInstance>,27>items{};wstring name;int openCount=0;float openness=0,oOpenness=0;
 virtual wstring getName(){return name;}wstring getCustomName(){return name;}bool hasCustomName(){return !name.empty();}void setCustomName(wstring n){name=n;}
 virtual bool canPlaceItem(int,shared_ptr<ItemInstance>){return true;}virtual void setItem(unsigned s,shared_ptr<ItemInstance>i){items.at(s)=i;}shared_ptr<ItemInstance>getItem(unsigned s){return items.at(s);}
 virtual void checkNeighbors(){}virtual void startOpen(){}virtual void stopOpen(){}shared_ptr<TileEntity>clone()override{return nullptr;}
};
struct Tile {
 enum{SHAPE_ENTITYTILE_ANIMATED=22,UPDATE_ALL=3};inline static Tile*tiles[256]={};int id;Icon*icon=nullptr;explicit Tile(int i):id(i){tiles[i]=this;}virtual ~Tile()=default;
 virtual bool isSolidRender(bool=false){return true;}virtual bool isCubeShaped(){return true;}virtual int getRenderShape(){return 0;}virtual int getResourceCount(Random*){return 1;}virtual int getPistonPushReaction(){return 0;}
 virtual shared_ptr<TileEntity>newTileEntity(Level*){return nullptr;}virtual int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int){return 0;}
 virtual void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>){}virtual bool TestUse(){return false;}
 virtual bool use(Level*,int,int,int,shared_ptr<Player>,int,float,float,float,bool=false){return false;}virtual void playerWillDestroy(Level*,int,int,int,int,shared_ptr<Player>){}
 virtual void onRemove(Level*,int,int,int,int,int){}virtual AABB*getAABB(Level*,int,int,int){return nullptr;}virtual void registerIcons(IconRegister*){}virtual bool hasAnalogOutputSignal(){return false;}virtual int getAnalogOutputSignal(Level*,int,int,int,int){return 0;}
 void setLightBlock(int){}void popResource(Level*l,int,int,int,shared_ptr<ItemInstance>i){l->drops.push_back(i);}
};
struct BaseEntityTile:Tile {BaseEntityTile(int i,Material*,bool):Tile(i){}void onRemove(Level*l,int x,int y,int z,int,int)override{l->removeTileEntity(x,y,z);}};
inline bool Level::setTileAndData(int x,int y,int z,int id,int metadata,int){if(!allowSet)return false;worldTile=Tile::tiles[id];te=worldTile->newTileEntity(this);te->level=this;te->x=x;te->y=y;te->z=z;data=metadata;placedX=x;placedY=y;placedZ=z;return true;}
struct ContainerOpenPacket {enum {SHULKER_BOX=18};};
struct AbstractContainerMenu {static int getRedstoneSignalFromContainer(shared_ptr<Container>){return 0;}};
struct HtmlString {wstring text;explicit HtmlString(wstring s):text(s){}};
struct TileItem {explicit TileItem(int){}void setMaxStackSize(int){}virtual void appendHoverText(shared_ptr<ItemInstance>,shared_ptr<Player>,vector<HtmlString>*,bool){}};
