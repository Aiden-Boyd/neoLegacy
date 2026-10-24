"""Compile production round state, server controller, share codes and import lookup."""
from pathlib import Path
import binascii,json,tempfile
from base_stability_regression import compile_and_run
ROOT=Path(__file__).resolve().parents[1];M=ROOT/'Minecraft.Client/Common/MiniGames'
CORE=r'''
#include <cassert>
#include <string>
int main(){
 using namespace Tumble;Round r;r.tick({1});assert(r.phase==Phase::Waiting);r.tick({1,2});assert(r.phase==Phase::Preparing);r.ready();for(int i=0;i<300;++i)r.tick({1,2});assert(r.canPlay(1)&&r.canPlay(2));r.tick({1,2,3});assert(!r.canPlay(3));r.tick({1,2,3},{1,2});assert(r.phase==Phase::Results&&r.winner==-1&&r.wins.empty());
 for(int n=0;n<3;++n){r= n==0?Round():r;r.start();r.ready();for(int i=0;i<300;++i)r.tick({1,2});r.tick({1});assert(r.winner==1&&r.wins[1]==n+1);if(n==2)assert(r.matchWinner==1);for(int i=0;i<160;++i)r.tick({1,2});assert(r.phase==Phase::Waiting);}assert(r.wins.empty());
 r.start(true);r.ready();for(int i=0;i<300;++i)r.tick({1});assert(r.phase==Phase::Playing);for(int i=0;i<3599;++i)r.tick({1});assert(r.phase==Phase::Playing);r.tick({1});assert(r.phase==Phase::Results&&r.winner==-1&&r.wins.empty());
 r=Round();r.start();r.ready();r.tick({1});assert(r.phase==Phase::Waiting);r.start();r.ready();for(int i=0;i<300;++i)r.tick({1,2,3,4,5,6,7,8,9});assert(r.alive.size()==8&&!r.canPlay(9));
 unsigned char text[]="123456789";assert(JoinCode::crc(text,9)==0x29b1);
 for(auto host:{L"127.0.0.1",L"192.168.1.7",L"203.0.113.9"})for(int p:{1,25565,65535}){auto code=JoinCode::encode(host,p);assert(!code.empty());std::wstring decoded=L"unchanged";int port=17;assert(JoinCode::decode(code,decoded,port)&&decoded==host&&port==p);for(size_t i=5;i<code.size();++i)if(code[i]!=L'-'){auto bad=code;bad[i]=bad[i]==L'0'?L'1':L'0';std::wstring untouched=L"untouched";int n=7;assert(!JoinCode::decode(bad,untouched,n)&&untouched==L"untouched"&&n==7);}}
 for(auto host:{L"0.0.0.0",L"256.1.1.1",L"1.2.3",L"1.2.3.4garbage",L"224.1.1.1",L"example.com"})assert(JoinCode::encode(host,25565).empty());assert(JoinCode::encode(L"127.0.0.1",0).empty()&&JoinCode::encode(L"127.0.0.1",65536).empty());
 assert(LegacyItemNames::id(L"minecraft:elytra")==443&&LegacyItemNames::id(L"minecraft:record_far")==2260&&LegacyItemNames::id(L"golden_chestplate")==315&&LegacyItemNames::id(L"4")==4&&LegacyItemNames::id(L"mod:unknown")==0&&LegacyItemNames::id(L"99999999999999999")==0);
 assert(LegacyEntityNames::entity(L"minecraft:horse")==L"EntityHorse"&&LegacyEntityNames::tile(L"minecraft:chest")==L"Chest"&&LegacyEntityNames::entity(L"Unknown")==L"Unknown");
}
'''
SESSION=r'''
int main(){
 GameType::SURVIVAL=&survival;GameType::ADVENTURE=&adventure;
 MinecraftServer s;auto a=make_shared<ServerPlayer>(1),b=make_shared<ServerPlayer>(2);a->server=b->server=&s;s.list.players={a,b};TumbleSession session(&s);session.tick();assert(s.world.writes==0&&a->inventory->clears==0);session.enable(true);
 auto ticks=[&](int n){for(int i=0;i<n;++i)session.tick();};ticks(350);assert(a->mode==GameType::SURVIVAL&&a->y==201&&b->y==201&&a->inventory->items[0]->id==277);
 int cx=-247,cz=-245;a->x=cx+.5;a->z=cz+.5;assert(session.breakBlock(a,&s.world,cx,200,cz)&&s.world.getTile(cx,200,cz)==0);assert(!session.breakBlock(a,&s.world,-343,57,-341));a->x=cx+20;assert(!session.breakBlock(a,&s.world,cx+1,200,cz));a->x=cx;
 auto late=make_shared<ServerPlayer>(3);late->server=&s;s.list.players.push_back(late);ticks(1);assert(late->mode==GameType::ADVENTURE&&late->y==206);assert(!session.breakBlock(late,&s.world,cx,194,cz));
 a->y=b->y=183;ticks(1);assert(a->mode==GameType::ADVENTURE&&a->y==206);assert(a->messages.back().find(L"Draw")!=wstring::npos);ticks(350);assert(s.world.getTile(cx,200,cz)==80);
 assert(session.command(a,L"/tumble stop"));int clears=a->inventory->clears;ticks(400);assert(a->mode==GameType::ADVENTURE&&a->inventory->clears==clears);b->op=false;assert(session.command(b,L"/tumble practice"));assert(b->messages.back().find(L"Only the host")!=wstring::npos);
 assert(session.command(a,L"/tumble start"));ticks(350);assert(a->mode==GameType::SURVIVAL);
 s.world.blocks[{cx,180,cz}]=10;TumbleSession restarted(&s);restarted.enable(true);for(int i=0;i<350;++i)restarted.tick();assert(a->mode==GameType::SURVIVAL);
 MinecraftServer solo;auto one=make_shared<ServerPlayer>(4);one->server=&solo;solo.list.players={one};TumbleSession practice(&solo);practice.enable(true);practice.tick();assert(practice.command(one,L"/tumble practice"));for(int i=0;i<350;++i)practice.tick();assert(one->y==201&&one->mode==GameType::SURVIVAL);one->y=183;practice.tick();assert(one->messages.back().find(L"Practice finished")!=wstring::npos);
 MinecraftServer occupied;occupied.list.players={make_shared<ServerPlayer>(5),make_shared<ServerPlayer>(6)};occupied.world.blocks[{cx,195,cz}]=1;TumbleSession blocked(&occupied);blocked.enable(true);for(int i=0;i<40;++i)blocked.tick();assert(occupied.world.writes==0&&occupied.world.getTile(cx,195,cz)==1);assert(occupied.list.players[0]->messages.back().find(L"occupied")!=wstring::npos);
 MinecraftServer failed;failed.world.fail=true;failed.list.players={make_shared<ServerPlayer>(7),make_shared<ServerPlayer>(8)};TumbleSession noWrites(&failed);noWrites.enable(true);for(int i=0;i<350;++i)noWrites.tick();assert(failed.list.players[0]->mode==GameType::ADVENTURE&&failed.list.players[0]->messages.back().find(L"reset failed")!=wstring::npos);
}
'''
def main():
 with tempfile.TemporaryDirectory()as d:
  p=Path(d);headers=[M/'TumbleRound.h',ROOT/'Minecraft.Client/Common/Network/JoinCode.h',ROOT/'Minecraft.World/LegacyItemNames.h',ROOT/'Minecraft.World/LegacyEntityNames.h'];code='\n'.join(x.read_text()for x in headers)+CORE;compile_and_run(p,'round_codes_import_names',code)
  (p/'TumbleRound.h').write_bytes((M/'TumbleRound.h').read_bytes());(p/'TumbleSession.h').write_bytes((M/'TumbleSession.h').read_bytes());source='\n'.join(l for l in(M/'TumbleSession.cpp').read_text().splitlines()if not l.startswith('#include'))
  compile_and_run(p,'tumble_server_session',(ROOT/'tests/tumble_fixture.h').read_text()+'\n#include "TumbleSession.h"\n'+source+SESSION)
 m=json.loads((ROOT/'tests/import_registry_sources.json').read_text());assert len(m['entries'])==462
 for ui in ['UIScene_LoadCreateJoinMenu.cpp','UIScene_LoadOrJoinMenu.cpp','UIScene_JoinMenu.cpp']:assert 'JoinCode::decode(wBuf,decodedHost,decodedPort)'in(ROOT/'Minecraft.Client/Common/UI'/ui).read_text()
 print('Native round/server transitions, protection, late joins, resets, practice, codes and import registry: passed')
if __name__=='__main__':main()
