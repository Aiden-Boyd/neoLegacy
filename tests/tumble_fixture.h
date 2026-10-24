#pragma once
#include <cassert>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
struct GameType{int id;inline static GameType *SURVIVAL,*ADVENTURE;};inline GameType survival{0},adventure{2};
struct ItemInstance{int id,count,aux;ItemInstance(int i,int n,int a):id(i),count(n),aux(a){}};
struct Inventory{vector<shared_ptr<ItemInstance>>items=vector<shared_ptr<ItemInstance>>(40);int selected=0,clears=0;int clearInventory(int,int){++clears;for(auto&i:items)i=nullptr;return 0;}void setItem(unsigned s,shared_ptr<ItemInstance>i){items.at(s)=i;}void setChanged(){}};
struct Slot{int index=36;};struct AbstractContainerMenu{int containerId=0;Slot slot;Slot*getSlotFor(shared_ptr<Inventory>,int){return &slot;}};
struct LevelData{int getXSpawn(){return -343;}int getYSpawn(){return 57;}int getZSpawn(){return -341;}};
struct Level{map<tuple<int,int,int>,int>blocks;bool fail=false;int writes=0;LevelData data;virtual~Level()=default;int getTile(int x,int y,int z){return blocks[{x,y,z}];}bool setTileAndUpdate(int x,int y,int z,int id){if(fail)return false;blocks[{x,y,z}]=id;++writes;return true;}void getChunk(int,int){}LevelData*getLevelData(){return &data;}};struct ServerLevel:Level{};
struct Packet{virtual~Packet()=default;};struct TileUpdatePacket:Packet{TileUpdatePacket(int,int,int,Level*){}};struct ContainerSetSlotPacket:Packet{ContainerSetSlotPacket(int,int,shared_ptr<ItemInstance>){}};
struct ServerPlayer;struct PlayerConnection{ServerPlayer*player;int packets=0;void send(shared_ptr<Packet>){++packets;}void teleport(double,double,double,float,float);};
struct MinecraftServer;
struct ServerPlayer{MinecraftServer*server=nullptr;int entityId,dimension=0;bool removed=false,op=true;double x=-343,y=57,z=-341,xd=0,yd=0,zd=0;float fallDistance=0,health=20;wstring name;struct{bool invulnerable=false,flying=false,mayfly=false,instabuild=false;}abilities;struct{void setFoodLevel(int){}void setSaturation(float){}void setExhaustion(float){}}foodData;shared_ptr<Inventory>inventory=make_shared<Inventory>();AbstractContainerMenu menu,*inventoryMenu=&menu;shared_ptr<PlayerConnection>connection=make_shared<PlayerConnection>();vector<wstring>messages;GameType*mode=nullptr;explicit ServerPlayer(int id):entityId(id),name(L"Player"+to_wstring(id)){connection->player=this;}void sendMessage(wstring m){messages.push_back(m);}void setGameMode(GameType*m){mode=m;}void onUpdateAbilities(){}void setHealth(float n){health=n;}float getHealth(){return health;}float getMaxHealth(){return 20;}void removeAllEffects(){}void refreshContainer(AbstractContainerMenu*){}};
inline void PlayerConnection::teleport(double x,double y,double z,float,float){player->x=x;player->y=y;player->z=z;}
struct PlayerList{vector<shared_ptr<ServerPlayer>>players;auto getPlayersSnapshot(){return players;}void toggleDimension(shared_ptr<ServerPlayer>p,int n){p->dimension=n;}bool isOp(shared_ptr<ServerPlayer>p){return p->op;}};
struct MinecraftServer{ServerLevel world;PlayerList list;ServerLevel*getLevel(int){return &world;}PlayerList*getPlayers(){return &list;}};
struct Tile{inline static Tile*tiles[256];bool use(Level*,int,int,int,shared_ptr<ServerPlayer>,int,float,float,float){return true;}};
