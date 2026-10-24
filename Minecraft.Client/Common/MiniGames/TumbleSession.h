#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "TumbleRound.h"
class MinecraftServer;class ServerPlayer;class Level;
class TumbleSession {
 MinecraftServer *server;
 bool enabledFlag=false,layoutReady=false,failed=false,paused=false;
 int lobbyX=0,lobbyY=0,lobbyZ=0,centerX=0,centerZ=0;
 struct Cell{int x,y,z,tile;};std::vector<Cell> build;size_t buildAt=0;
 Tumble::Round round;std::set<int> known;
 void tell(const std::wstring&);
 void move(std::shared_ptr<ServerPlayer>,int role,int index=0);
 bool prepare();
public:
 explicit TumbleSession(MinecraftServer*s):server(s){}
 void enable(bool b){enabledFlag=b;}
 bool enabled()const{return enabledFlag;}
 void tick();
 bool breakBlock(std::shared_ptr<ServerPlayer>,Level*,int,int,int);
 bool useTile(std::shared_ptr<ServerPlayer>,Level*,int,int,int,int,float,float,float,bool);
 bool command(std::shared_ptr<ServerPlayer>,const std::wstring&);
};
