#include "stdafx.h"
#include "TumbleSession.h"
#include "../../MinecraftServer.h"
#include "../../ServerLevel.h"
#include "../../ServerPlayer.h"
#include "../../PlayerList.h"
#include "../../PlayerConnection.h"
#include "../../../Minecraft.World/LevelData.h"
#include "../../../Minecraft.World/Inventory.h"
#include "../../../Minecraft.World/ItemInstance.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"
#include "../../../Minecraft.World/LevelSettings.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include <cmath>
void TumbleSession::tell(const std::wstring&text){for(auto p:server->getPlayers()->getPlayersSnapshot())if(p->connection&&!p->removed)p->sendMessage(L"[Tumble] "+text);}
void TumbleSession::move(std::shared_ptr<ServerPlayer>p,int role,int index){
 if(!p->connection)return;
 if(p->dimension!=0)server->getPlayers()->toggleDimension(p,0);
 p->inventory->clearInventory(-1,-1);p->inventory->selected=0;p->setGameMode(role==1?GameType::SURVIVAL:GameType::ADVENTURE);
 p->abilities.invulnerable=true;p->abilities.flying=false;p->abilities.mayfly=false;p->abilities.instabuild=false;p->onUpdateAbilities();
 p->setHealth(p->getMaxHealth());p->foodData.setFoodLevel(20);p->foodData.setSaturation(5);p->foodData.setExhaustion(0);p->removeAllEffects();p->fallDistance=0;p->xd=p->yd=p->zd=0;
 if(role==1){
  static const int dx[]={-7,7,-7,7,0,0,-7,7},dz[]={-7,7,7,-7,-7,7,0,0};index%=8;
  p->inventory->setItem(0,std::make_shared<ItemInstance>(277,1,0));p->connection->teleport(centerX+dx[index]+.5,201,centerZ+dz[index]+.5,0,0);
 }else if(role==2)p->connection->teleport(centerX+17.5,206,centerZ+.5,90,15);
 else p->connection->teleport(lobbyX+.5,lobbyY,lobbyZ+.5,0,0);
 p->inventory->setChanged();p->refreshContainer(p->inventoryMenu);
}
bool TumbleSession::prepare(){
 auto level=server->getLevel(0);
 // Explicitly opted-in lobby copies reserve this sky volume. Never erase scenery.
 for(int x=(centerX-13)>>4;x<=(centerX+20)>>4;++x)for(int z=(centerZ-13)>>4;z<=(centerZ+13)>>4;++z)level->getChunk(x,z);
 if(!layoutReady){
  for(int x=centerX-13;x<=centerX+20;++x)for(int z=centerZ-13;z<=centerZ+13;++z)for(int y=179;y<=208;++y){int id=level->getTile(x,y,z);if(id!=0&&id!=80&&id!=20&&id!=49&&id!=11)return false;}
  layoutReady=true;
 }
 build.clear();buildAt=0;
 for(int x=-12;x<=12;++x)for(int z=-12;z<=12;++z){build.push_back({centerX+x,179,centerZ+z,49});build.push_back({centerX+x,180,centerZ+z,(std::abs(x)==12||std::abs(z)==12)?49:11});}
 for(int y:{188,194,200})for(int x=-10;x<=10;++x)for(int z=-10;z<=10;++z)build.push_back({centerX+x,y,centerZ+z,80});
 for(int x=15;x<=19;++x)for(int z=-2;z<=2;++z){build.push_back({centerX+x,205,centerZ+z,20});if(x==15||x==19||z==-2||z==2)for(int y:{206,207})build.push_back({centerX+x,y,centerZ+z,20});}
 return true;
}
void TumbleSession::tick(){
 if(!enabledFlag||failed)return;auto level=server->getLevel(0);if(!level||!server->getPlayers())return;
 if(!layoutReady&&build.empty()){auto data=level->getLevelData();lobbyX=data->getXSpawn();lobbyY=data->getYSpawn();lobbyZ=data->getZSpawn();centerX=lobbyX+96;centerZ=lobbyZ+96;}
 auto players=server->getPlayers()->getPlayersSnapshot();std::vector<int> connected;std::set<int> fallen,present;
 for(auto p:players)if(p->connection&&!p->removed){
  int id=p->entityId;connected.push_back(id);present.insert(id);
  if(!known.count(id)){known.insert(id);move(p,round.phase==Tumble::Phase::Playing||round.phase==Tumble::Phase::Results?2:0);p->sendMessage(L"[Tumble] Shovel Tumble: remove snow under other players. Two players start automatically. Host: /tumble practice for a solo test.");}
  if(round.canPlay(id)&&(p->dimension!=0||p->y<184||std::abs(p->x-centerX)>13||std::abs(p->z-centerZ)>13||p->getHealth()<=0))fallen.insert(id);
  if(round.phase==Tumble::Phase::Playing&&!round.canPlay(id)&&(!p->abilities.invulnerable||p->dimension!=0||std::abs(p->x-(centerX+17))>3||std::abs(p->z-centerZ)>4||p->y<205||p->y>212))move(p,2);
  if(round.phase!=Tumble::Phase::Playing&&round.phase!=Tumble::Phase::Results&&(p->dimension!=0||p->y<1))move(p,0);
  // Host-option changes cannot grant flight or Creative destruction during a round.
  if(p->abilities.mayfly||p->abilities.flying||p->abilities.instabuild||!p->abilities.invulnerable){p->abilities.mayfly=p->abilities.flying=p->abilities.instabuild=false;p->abilities.invulnerable=true;p->onUpdateAbilities();}
  p->fallDistance=0;p->foodData.setFoodLevel(20);p->foodData.setExhaustion(0);
 }
 for(auto it=known.begin();it!=known.end();)if(!present.count(*it))it=known.erase(it);else ++it;
 auto before=round.phase;auto oldAlive=round.alive;
 if(round.phase==Tumble::Phase::Preparing){
  if(build.empty()&&!prepare()){failed=true;tell(L"Arena space is occupied. No scenery was removed; use the supplied fresh lobby copy.");return;}
  int budget=128;while(buildAt<build.size()&&budget-->0){auto c=build[buildAt++];if(level->getTile(c.x,c.y,c.z)!=c.tile&&!level->setTileAndUpdate(c.x,c.y,c.z,c.tile)){failed=true;tell(L"Arena reset failed; the round has not started.");return;}}
  if(buildAt==build.size()){build.clear();round.ready();tell(L"Round starts in 15 seconds. First to three wins.");}
 }else if(!paused||round.phase!=Tumble::Phase::Waiting)round.tick(connected,fallen);
 if(before!=round.phase){
  if(round.phase==Tumble::Phase::Preparing){for(auto p:players)if(p->connection&&!p->removed)move(p,0);}
  else if(round.phase==Tumble::Phase::Playing){int slot=0;for(auto p:players)if(p->connection&&!p->removed)move(p,round.canPlay(p->entityId)?1:2,round.canPlay(p->entityId)?slot++:0);tell(round.practice?L"Solo practice started. Falling ends the test.":L"Go! Remove snow with your shovel.");}
  else if(round.phase==Tumble::Phase::Results){std::wstring name;for(auto p:players)if(p->entityId==round.winner)name=p->name;std::wstring text=round.winner<0?(round.practice?L"Practice finished.":L"Draw."):name+L" wins the round! ("+std::to_wstring(round.wins[round.winner])+L"/3)";if(round.matchWinner>=0)text=name+L" wins the match!";tell(text);for(auto p:players)if(p->connection&&!p->removed)move(p,2);}
  else if(round.phase==Tumble::Phase::Waiting){for(auto p:players)if(p->connection&&!p->removed)move(p,0);}
 }
 if(round.phase==Tumble::Phase::Playing){for(auto p:players)if(p->connection&&oldAlive.count(p->entityId)&&!round.alive.count(p->entityId))move(p,2);if(round.ticks%20==0&&round.ticks<=200)tell(std::to_wstring(round.ticks/20)+L" seconds left.");}
 if(round.phase==Tumble::Phase::Countdown&&round.ticks>0&&round.ticks<=100&&round.ticks%20==0)tell(std::to_wstring(round.ticks/20)+L"...");
}
bool TumbleSession::breakBlock(std::shared_ptr<ServerPlayer>p,Level*l,int x,int y,int z){
 bool allowed=l==server->getLevel(0)&&p->dimension==0&&round.canPlay(p->entityId)&&std::abs(x-centerX)<=10&&std::abs(z-centerZ)<=10&&(y==188||y==194||y==200)&&l->getTile(x,y,z)==80;
 double dx=p->x-(x+.5),dy=p->y+1.62-(y+.5),dz=p->z-(z+.5);allowed=allowed&&dx*dx+dy*dy+dz*dz<=36;
 if(allowed){bool removed=l->setTileAndUpdate(x,y,z,0);if(removed&&p->connection){auto slot=p->inventoryMenu->getSlotFor(p->inventory,0);if(slot)p->connection->send(std::make_shared<ContainerSetSlotPacket>(p->inventoryMenu->containerId,slot->index,p->inventory->items[0]));}return removed;}
 if(p->connection)p->connection->send(std::make_shared<TileUpdatePacket>(x,y,z,l));return false;
}
bool TumbleSession::useTile(std::shared_ptr<ServerPlayer>p,Level*l,int x,int y,int z,int face,float a,float b,float c,bool preview){
 if(round.phase==Tumble::Phase::Playing||round.phase==Tumble::Phase::Results||p->dimension!=0)return false;int id=l->getTile(x,y,z);
 bool door=id==64||id==71||(id>=193&&id<=197)||id==96||id==167||id==107||(id>=183&&id<=187)||id==69||id==77||id==143;
 if(!door||!Tile::tiles[id])return false;return preview||Tile::tiles[id]->use(l,x,y,z,p,face,a,b,c);
}
bool TumbleSession::command(std::shared_ptr<ServerPlayer>p,const std::wstring&message){
 if(message!=L"/tumble"&&message.compare(0,8,L"/tumble ")!=0)return false;
 if(!enabledFlag){p->sendMessage(L"Load the marked Tumble lobby world first.");return true;}
 if(message==L"/tumble"){p->sendMessage(L"Tumble: 2+ players start automatically; host /tumble practice, /tumble start or /tumble stop.");return true;}
 if(!server->getPlayers()->isOp(p)){p->sendMessage(L"Only the host can control rounds.");return true;}
 if(message==L"/tumble practice"){paused=false;if(!round.start(true))p->sendMessage(L"Wait for the current round to finish.");}
 else if(message==L"/tumble start"){paused=false;if(round.phase!=Tumble::Phase::Waiting)p->sendMessage(L"A round is already in progress.");}
 else if(message==L"/tumble stop"){paused=true;round=Tumble::Round();build.clear();buildAt=0;for(auto player:server->getPlayers()->getPlayersSnapshot())if(player->connection&&!player->removed)move(player,0);tell(L"Round stopped. Return to the lobby.");}
 else p->sendMessage(L"Use /tumble practice, /tumble start or /tumble stop.");return true;
}
