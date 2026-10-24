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
#define _CONTENT_PACKAGE
#define PIXBeginNamedEvent(...)
#define PIXEndNamedEvent(...)
struct Material {bool liquid=false;static Material*stone,*sand,*water,*lava;bool isLiquid(){return liquid;}};
inline Material rock{},sandMaterial{},waterMaterial{true},lavaMaterial{true};inline Material*Material::stone=&rock,*Material::sand=&sandMaterial,*Material::water=&waterMaterial,*Material::lava=&lavaMaterial;
struct Icon{wstring name;};struct IconRegister{vector<unique_ptr<Icon>>icons;Icon*registerIcon(wstring name){icons.push_back(make_unique<Icon>(Icon{name}));return icons.back().get();}};
struct Facing{inline static int STEP_X[6]={0,0,0,0,-1,1},STEP_Y[6]={-1,1,0,0,0,0},STEP_Z[6]={0,0,-1,1,0,0};};
struct Random{};struct Mth{static int floor(double n){return int(std::floor(n));}};
struct Level;using LevelSource=Level;struct LivingEntity{double x=0,y=64,z=0,heightOffset=0;float yRot=0;};struct ItemInstance{};
struct Tile {
 enum{observer_Id=218,glazedTerracottaFirst_Id=235,concrete_Id=251,concretePowder_Id=252,UPDATE_NEIGHBORS=1,UPDATE_CLIENTS=2,UPDATE_INVISIBLE=4,UPDATE_INVISIBLE_NO_LIGHT=12,UPDATE_SUPPRESS_OBSERVERS=16,UPDATE_ALL=3,TILE_NUM_MASK=4095};
 inline static Tile*tiles[4096]={};inline static unsigned char _sendTileData[4096]={};int id;Material*material;Icon*icon=nullptr;
 Tile(int id,Material*m):id(id),material(m){tiles[id]=this;}virtual~Tile(){};
 void sendTileData(int mask=15){_sendTileData[id]=mask;}virtual void registerIcons(IconRegister*){}virtual Icon*getTexture(int,int){return icon;}virtual int getSpawnResourcesAuxValue(int){return 0;}
 virtual void onPlace(Level*,int,int,int){}virtual void neighborChanged(Level*,int,int,int,int){}virtual void tick(Level*,int,int,int,Random*){}virtual void onLand(Level*,int,int,int,int){}
 virtual void onRemove(Level*,int,int,int,int,int){}virtual int getPistonPushReaction(){return 0;}virtual void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>){}virtual bool isSignalSource(){return false;}virtual int getSignal(LevelSource*,int,int,int,int){return 0;}virtual int getDirectSignal(LevelSource*,int,int,int,int){return 0;}
 bool hasAnalogOutputSignal(){return false;}
};
struct Entity {Level*level;bool removed=false;double x=0,y=0,z=0,xo=0,yo=0,zo=0;Entity(Level*l):level(l){}virtual~Entity(){};void remove(){removed=true;}};
struct FallingTile:Entity {int tile,data;FallingTile(Level*l,double x,double y,double z,int id,int d):Entity(l),tile(id),data(d){this->x=x;this->y=y;this->z=z;xo=x;yo=y;zo=z;}};
struct HeavyTile:Tile {inline static bool instaFall=false;HeavyTile(int id):Tile(id,Material::sand){}virtual void onLand(Level*,int,int,int,int){}void onPlace(Level*,int,int,int);void neighborChanged(Level*,int,int,int,int);static bool isFree(Level*,int,int,int);};
using Position=tuple<int,int,int>;struct LevelChunk;
struct Level {
 enum{maxBuildHeight=256,MAX_LEVEL_SIZE=2048};bool isClientSide=false,loaded=true,allowFall=true,failWrite=false,failSpawn=false;int clock=0,writes=0,notices=0;
 map<Position,int>blocks,data;map<tuple<int,int,int,int>,int>pending;vector<shared_ptr<Entity>>entities;map<pair<int,int>,unique_ptr<LevelChunk>>chunks;
 int getTile(int x,int y,int z){auto p=blocks.find({x,y,z});return p==blocks.end()?0:p->second;}int getData(int x,int y,int z){return data[{x,y,z}];}
 Material*getMaterial(int x,int y,int z){int id=getTile(x,y,z);if(id==8||id==9)return Material::water;if(id==10||id==11)return Material::lava;return id&&Tile::tiles[id]?Tile::tiles[id]->material:Material::stone;}
 bool hasChunkAt(int,int,int){return loaded;}bool hasChunksAt(int,int,int,int,int,int){return loaded;}bool newFallingTileAllowed(){return allowFall;}
 bool addEntity(shared_ptr<Entity>e){if(failSpawn)return false;entities.push_back(e);return true;}
 bool setTileAndData(int,int,int,int,int,int);bool setData(int,int,int,int,int,bool=false);bool removeTile(int x,int y,int z){return setTileAndData(x,y,z,0,0,3);}
 LevelChunk*getChunk(int,int);void checkLight(int,int,int){}void sendTileUpdated(int,int,int){}void tileUpdated(int,int,int,int){}void updateNeighbourForOutputSignal(int,int,int,int){}
 void addToTickNextTick(int x,int y,int z,int id,int delay){pending.emplace(make_tuple(x,y,z,id),clock+delay);}bool isTileToBeTickedAt(int x,int y,int z,int id){return pending.count({x,y,z,id});}
 void neighborChanged(int,int,int,int){++notices;}void updateNeighborsAtExceptFromFacing(int,int,int,int,int){++notices;}
 void advance(int n){for(int k=0;k<n;++k){++clock;vector<tuple<int,int,int,int>>due;for(auto t:pending)if(t.second<=clock)due.push_back(t.first);for(auto t:due){pending.erase(t);auto[x,y,z,id]=t;if(getTile(x,y,z)==id)Tile::tiles[id]->tick(this,x,y,z,nullptr);}}}
};
struct LevelChunk {Level*l;int cx,cz;int getTile(int x,int y,int z){return l->getTile(cx*16+x,y,cz*16+z);}int getData(int x,int y,int z){return l->getData(cx*16+x,y,cz*16+z);}
 bool setTileAndData(int x,int y,int z,int id,int d){x+=cx*16;z+=cz*16;int old=l->getTile(x,y,z),oldData=l->getData(x,y,z);if(l->failWrite||(old==id&&oldData==d))return false;l->blocks[{x,y,z}]=id;l->data[{x,y,z}]=d;++l->writes;if(old!=id){if(old&&Tile::tiles[old])Tile::tiles[old]->onRemove(l,x,y,z,old,oldData);if(id&&Tile::tiles[id])Tile::tiles[id]->onPlace(l,x,y,z);}return true;}
 bool setData(int x,int y,int z,int d,int,bool*changed){x+=cx*16;z+=cz*16;*changed=l->getData(x,y,z)!=d;if(l->failWrite||!*changed)return false;l->data[{x,y,z}]=d;++l->writes;return true;}
};
inline LevelChunk*Level::getChunk(int x,int z){auto&p=chunks[{x,z}];if(!p)p=make_unique<LevelChunk>(LevelChunk{this,x,z});return p.get();}
inline void HeavyTile::onPlace(Level*l,int x,int y,int z){l->addToTickNextTick(x,y,z,id,2);}inline void HeavyTile::neighborChanged(Level*l,int x,int y,int z,int){onPlace(l,x,y,z);}inline bool HeavyTile::isFree(Level*l,int x,int y,int z){int id=l->getTile(x,y,z);return id==0||id==51||id==8||id==9||id==10||id==11;}
