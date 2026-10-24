#pragma once
#include <algorithm>
#include <map>
#include <set>
#include <vector>
// Server-owned round state. A tick is 1/20 second; no client can declare a win.
namespace Tumble {
enum class Phase { Waiting, Preparing, Countdown, Playing, Results };
struct Round {
 Phase phase=Phase::Waiting;
 int ticks=0,winner=-1,matchWinner=-1;
 bool practice=false;
 std::set<int> roster,alive;
 std::map<int,int> wins;
 static const int COUNTDOWN=300,LIMIT=3600,RESULTS=160;
 bool start(bool solo=false){if(phase!=Phase::Waiting)return false;practice=solo;phase=Phase::Preparing;ticks=0;winner=matchWinner=-1;return true;}
 void ready(){if(phase==Phase::Preparing){phase=Phase::Countdown;ticks=COUNTDOWN;}}
 void finish(int id){winner=id;matchWinner=-1;if(id>=0&&!practice&&++wins[id]>=3)matchWinner=id;phase=Phase::Results;ticks=RESULTS;}
 void tick(const std::vector<int>&connected,const std::set<int>&fallen={}) {
  std::set<int> present(connected.begin(),connected.end());
  for(auto it=wins.begin();it!=wins.end();)if(!present.count(it->first))it=wins.erase(it);else ++it;
  if(phase==Phase::Waiting){if(present.size()>=2)start();return;}
  if(phase==Phase::Countdown){
   if(present.size()<size_t(practice?1:2)){phase=Phase::Waiting;ticks=0;return;}
   if(--ticks<=0){alive=present;while(alive.size()>8)alive.erase(--alive.end());roster=alive;phase=Phase::Playing;ticks=LIMIT;}return;
  }
  if(phase==Phase::Playing){
   // Apply every elimination before judging the result: simultaneous falls draw.
   for(auto it=alive.begin();it!=alive.end();)if(!present.count(*it)||fallen.count(*it))it=alive.erase(it);else ++it;
   if(alive.empty()){finish(-1);return;}
   if(!practice&&alive.size()==1){finish(*alive.begin());return;}
   if(--ticks<=0)finish(-1);return;
  }
  if(phase==Phase::Results&&--ticks<=0){if(matchWinner>=0)wins.clear();phase=Phase::Waiting;practice=false;roster.clear();alive.clear();}
 }
 bool canPlay(int id)const{return phase==Phase::Playing&&alive.count(id)!=0;}
};
}
