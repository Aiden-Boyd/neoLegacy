#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using eINSTANCEOF=int;
enum{eTYPE_ENDCITYBANNERENTITY};
struct Tag {enum{TAG_Int=3,TAG_String=8,TAG_List=9,TAG_Compound=10};virtual~Tag()=default;virtual int getId(){return TAG_Compound;}virtual Tag*copy()=0;};
template<class T>struct ListTag:Tag{vector<T*>values;~ListTag(){for(auto p:values)delete p;}int getId()override{return TAG_List;}int size(){return int(values.size());}T*get(int i){return values.at(i);}void add(T*p){values.push_back(p);}Tag*copy()override{auto r=new ListTag;for(auto p:values)r->add(static_cast<T*>(p->copy()));return r;}};
struct CompoundTag:Tag{
 map<wstring,int>numbers;map<wstring,wstring>strings;map<wstring,unique_ptr<Tag>>children;
 Tag*copy()override{auto r=new CompoundTag;r->numbers=numbers;r->strings=strings;for(auto&p:children)r->children[p.first].reset(p.second->copy());return r;}
 bool contains(wstring k){return numbers.count(k)||strings.count(k)||children.count(k);}
 bool contains(wstring k,int t){return t==99||t==TAG_Int?numbers.count(k):t==TAG_String?strings.count(k):children.count(k)&&children[k]->getId()==t;}
 void putInt(wstring k,int v){numbers[k]=v;}int getInt(wstring k){return numbers[k];}void putBoolean(wstring k,bool v){numbers[k]=v;}bool getBoolean(wstring k){return numbers[k]!=0;}
 void putString(wstring k,wstring v){strings[k]=v;}wstring getString(wstring k){return strings[k];}
 void put(wstring k,Tag*t){children[k].reset(t);}CompoundTag*getCompound(wstring k){return static_cast<CompoundTag*>(children.at(k).get());}ListTag<CompoundTag>*getList(wstring k){return static_cast<ListTag<CompoundTag>*>(children.at(k).get());}
};
struct ItemInstance{int id,count,aux;wstring name;unique_ptr<CompoundTag>tag;ItemInstance(int i,int n,int a):id(i),count(n),aux(a){}int getAuxValue(){return aux;}void setTag(CompoundTag*t){tag.reset(t);}bool hasTag(){return bool(tag);}CompoundTag*getTag(){return tag.get();}bool hasCustomHoverName(){return !name.empty();}void setHoverName(wstring s){name=s;}wstring getHoverName(){return name;}};
struct Material{bool solid,replaceable;enum{PUSH_DESTROY=1};static Material*wood,*clay;bool blocksMotion(){return solid;}bool isSolid(){return solid;}bool isReplaceable(){return replaceable;}};
inline Material solid{true,false},air{false,true};inline Material*Material::wood=&::solid;inline Material*Material::clay=&::solid;
struct AABB{double y0=64;};struct Random{};struct Level;
struct Entity{Level*level=nullptr;AABB bounds,*bb=&bounds;double x=0,z=0,xd=1,yd=-1,zd=1;float fallDistance=0,damage=0;bool sneak=false;virtual~Entity()=default;bool isSneaking(){return sneak;}void causeFallDamage(float n){damage+=n;}void checkFallDamage(double,bool);};
struct LivingEntity:Entity{};struct Player:LivingEntity{float yRot=0;bool allowed=true;struct{bool instabuild=false;}abilities;bool mayUseItemAt(int,int,int,int,shared_ptr<ItemInstance>){return allowed;}};
struct Packet{virtual~Packet()=default;};struct TileEntityDataPacket:Packet{enum{TYPE_BANNER=6};int type;unique_ptr<CompoundTag>tag;TileEntityDataPacket(int,int,int,int t,CompoundTag*p):type(t),tag(p){}};
struct TileEntity{Level*level=nullptr;int x=0,y=0,z=0,changes=0;virtual~TileEntity()=default;virtual eINSTANCEOF GetType(){return -1;}virtual void load(CompoundTag*t){x=t->getInt(L"x");y=t->getInt(L"y");z=t->getInt(L"z");}virtual void save(CompoundTag*t){t->putInt(L"x",x);t->putInt(L"y",y);t->putInt(L"z",z);}virtual shared_ptr<Packet>getUpdatePacket(){return nullptr;}virtual shared_ptr<TileEntity>clone()=0;void clone(shared_ptr<TileEntity>r){r->level=level;r->x=x;r->y=y;r->z=z;}void setChanged(){++changes;}};
struct Tile{enum{UPDATE_ALL=3,SHAPE_INVISIBLE=-1};inline static Tile*tiles[256];int id;float friction=0;Tile(int i,Material*,bool):id(i){tiles[i]=this;}virtual~Tile()=default;virtual bool isSolidRender(bool=false){return true;}virtual bool isCubeShaped(){return true;}virtual int getRenderLayer(){return 0;}virtual int getRenderShape(){return 0;}virtual int getResourceCount(Random*){return 1;}virtual int getPistonPushReaction(){return 0;}virtual AABB*getAABB(Level*,int,int,int){return nullptr;}virtual bool mayPlace(Level*,int,int,int,int){return true;}bool mayPlace(Level*,int,int,int){return true;}virtual int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int){return 0;}virtual void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>){}virtual void neighborChanged(Level*,int,int,int,int){}virtual void playerWillDestroy(Level*,int,int,int,int,shared_ptr<Player>){}virtual void onRemove(Level*,int,int,int,int,int){}virtual shared_ptr<TileEntity>newTileEntity(Level*){return nullptr;}virtual void fallOn(Level*,int,int,int,shared_ptr<Entity>,float){}void setLightBlock(int){}void popResource(Level*,int,int,int,shared_ptr<ItemInstance>);};
struct Level{
 static const int maxBuildHeight=256;using Pos=tuple<int,int,int>;map<Pos,int>blocks,data;map<Pos,shared_ptr<TileEntity>>entities;vector<shared_ptr<ItemInstance>>drops;bool isClientSide=false,loaded=true,fail=false;int updates=0,writes=0;
 int getTile(int x,int y,int z){return blocks[{x,y,z}];}int getData(int x,int y,int z){return data[{x,y,z}];}Material*getMaterial(int x,int y,int z){return getTile(x,y,z)?&solid:&air;}
 bool hasChunkAt(int,int,int){return loaded;}shared_ptr<TileEntity>getTileEntity(int x,int y,int z){return entities[{x,y,z}];}void removeTileEntity(int x,int y,int z){entities.erase({x,y,z});}void sendTileUpdated(int,int,int){++updates;}
 bool setTileAndData(int x,int y,int z,int id,int meta,int){if(fail)return false;++writes;auto old=getTile(x,y,z);if(old&&Tile::tiles[old])Tile::tiles[old]->onRemove(this,x,y,z,old,getData(x,y,z));blocks[{x,y,z}]=id;data[{x,y,z}]=meta;if(id&&Tile::tiles[id]){auto te=Tile::tiles[id]->newTileEntity(this);if(te){te->level=this;te->x=x;te->y=y;te->z=z;entities[{x,y,z}]=te;}}return true;}
 bool setTileAndUpdate(int x,int y,int z,int id){return setTileAndData(x,y,z,id,0,3);}
};
inline void Tile::popResource(Level*l,int,int,int,shared_ptr<ItemInstance>item){l->drops.push_back(item);}
struct BaseEntityTile:Tile{BaseEntityTile(int i,Material*m,bool b):Tile(i,m,b){}void onRemove(Level*l,int x,int y,int z,int,int)override{l->removeTileEntity(x,y,z);}};
struct Item{Item(int){}virtual~Item()=default;void setMaxStackSize(int){}void setStackedByData(bool){}virtual int getColor(shared_ptr<ItemInstance>,int){return 0;}virtual bool useOn(shared_ptr<ItemInstance>,shared_ptr<Player>,Level*,int,int,int,int,float,float,float,bool=false){return false;}};
struct Facing{inline static int STEP_X[6]={0,0,0,0,-1,1},STEP_Y[6]={-1,1,0,0,0,0},STEP_Z[6]={0,0,-1,1,0,0};};struct Mth{static int floor(double n){return int(std::floor(n));}};
