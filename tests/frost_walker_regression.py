"""Compile production Frost Walker, thawing and movement hook against a focused fixture."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
STUB=r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>
#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;
struct Icon {wstring name;};struct IconRegister {map<wstring,Icon> icons;Icon* registerIcon(const wchar_t* n){return &(icons[n]=Icon{n});}};
struct Random {int choice=0;int nextInt(int n){assert(n>0);return choice%n;}};
struct AABB {double y0=64;};
struct Item {inline static const int book_Id=340;};
struct ItemInstance {int id=1,count=1;bool boots=true;Item* getItem(){static Item i;return &i;} };
struct EnchantmentCategory {inline static const EnchantmentCategory *armor_feet=nullptr;bool canEnchant(Item*)const{return true;}};
struct Enchantment {
 int id,descriptionId;const EnchantmentCategory* category;
 inline static Enchantment *frostWalker=nullptr,*waterWalker=nullptr;
 struct Array {Enchantment* data[256]={};unsigned int length=256;Enchantment*&operator[](int i){return data[i];}};static Array enchantments;
 Enchantment(int i,int,const EnchantmentCategory*c):id(i),descriptionId(0),category(c){enchantments[id]=this;}
 virtual bool isCompatibleWith(Enchantment*)const;virtual bool isTreasureOnly()const{return false;}
 virtual int getMinCost(int){return 1;}virtual int getMaxCost(int){return 100;}virtual int getMinLevel(){return 1;}virtual int getMaxLevel(){return 1;}
 virtual bool canEnchant(shared_ptr<ItemInstance> i){return i->boots;}
 void setDescriptionId(int i){descriptionId=i;}
};inline Enchantment::Array Enchantment::enchantments;
struct EnchantmentInstance {Enchantment* enchantment;int level;EnchantmentInstance(Enchantment*e,int l):enchantment(e),level(l){}};
struct Entity:enable_shared_from_this<Entity>{virtual ~Entity()=default;};struct Level;
struct LivingEntity:Entity {
 enum{SLOT_BOOTS=1};double x=.5,z=.5;AABB box;AABB*bb=&box;bool onGround=true;Random rng;Random*random=&rng;Level*level=nullptr;
 bool frostPositionValid=false;int frostX=0,frostY=0,frostZ=0;vector<shared_ptr<ItemInstance>> equipment=vector<shared_ptr<ItemInstance>>(5);
 Random* getRandom(){return random;}
 shared_ptr<ItemInstance> getCarried(int slot){return equipment[slot];}void updateFrostWalking();
};
struct EnchantmentHelper {
 static int frostCalls;static int getEnchantmentLevel(int,shared_ptr<ItemInstance> p){++frostCalls;return p?2:0;}
 static unordered_map<int,EnchantmentInstance*>*getAvailableEnchantmentResults(int,shared_ptr<ItemInstance>,bool);
};int EnchantmentHelper::frostCalls=0;
struct Mth {static int floor(double d){return (int)std::floor(d);}};
struct Facing {enum{DOWN,UP,NORTH,SOUTH,WEST,EAST};inline static int STEP_X[]={0,0,0,0,-1,1},STEP_Y[]={-1,1,0,0,0,0},STEP_Z[]={0,0,-1,1,0,0};};
struct Dimension{bool ultraWarm=false;};using Pos=tuple<int,int,int>;
struct Level {
 enum{maxBuildHeight=256};bool isClientSide=false,loaded=true,allowPlace=true,failWrites=false;int brightness=15;Dimension dim;Dimension*dimension=&dim;
 map<Pos,int> blocks,data;vector<tuple<int,int,int,int,int>> scheduled;int writes=0,placements=0;
 int getTile(int x,int y,int z){return blocks[{x,y,z}];}int getData(int x,int y,int z){return data[{x,y,z}];}
 bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}bool hasChunkAt(int,int,int){return loaded;}
 bool mayPlace(int,int,int,int,bool ignore,int face,shared_ptr<Entity>,shared_ptr<ItemInstance>){assert(!ignore&&face==Facing::DOWN);++placements;return allowPlace;}
 bool setTileAndData(int x,int y,int z,int id,int aux,int){if(failWrites)return false;blocks[{x,y,z}]=id;data[{x,y,z}]=aux;++writes;return true;}
 void setTileAndUpdate(int x,int y,int z,int id){setTileAndData(x,y,z,id,0,3);}void removeTile(int x,int y,int z){setTileAndUpdate(x,y,z,0);}
 bool setData(int x,int y,int z,int aux,int flags){assert(flags==2);if(failWrites)return false;data[{x,y,z}]=aux;++writes;return true;}
 int getRawBrightness(int,int,int){return brightness;}
 void addToTickNextTick(int x,int y,int z,int id,int delay){scheduled.emplace_back(x,y,z,id,delay);}
};
struct Tile {enum{water_Id=8,calmWater_Id=9,frostedIce_Id=212,UPDATE_CLIENTS=2,UPDATE_ALL=3};inline static int lightBlock[256]={};int id;Icon*icon=nullptr;Tile(int i):id(i){} };
struct IceTile:Tile {float friction=.98f;IceTile(int id):Tile(id){} int getResourceCount(Random*);};
const int IDS_ENCHANTMENT_FROST_WALKER=1,IDS_ENCHANTMENT_WATER_WALKER=2;
'''
TEST=r'''
#include "FrostWalkerEnchantment.cpp"
#include "FrostedIceTile.cpp"
#include "WaterWalkerEnchantment.cpp"
void lake(Level& w,int yy=63){for(int x=-20;x<=20;++x)for(int z=-20;z<=20;++z)w.blocks[{x,yy,z}]=9;}
int frozen(Level&w){int n=0;for(auto b:w.blocks)n+=b.second==212;return n;}
void pool(int value,bool treasure,shared_ptr<ItemInstance> item,bool expected){auto result=EnchantmentHelper::getAvailableEnchantmentResults(value,item,treasure);assert((result&&result->count(9))==expected);if(result){for(auto i:*result)delete i.second;delete result;}}
int main(){
 EnchantmentCategory c;EnchantmentCategory::armor_feet=&c;FrostWalkerEnchantment f(9,2);WaterWalkerEnchantment d(8,2);Enchantment::frostWalker=&f;Enchantment::waterWalker=&d;
 assert(f.getMaxLevel()==2&&f.isTreasureOnly()&&f.getMinCost(1)==10&&f.getMinCost(2)==20&&f.getMaxCost(2)==35);
 assert(!f.isCompatibleWith(&d)&&!d.isCompatibleWith(&f)&&!f.isCompatibleWith(&f));
 auto boots=make_shared<ItemInstance>(),book=make_shared<ItemInstance>();book->id=340;book->boots=false;
 for(int v=1;v<80;++v){pool(v,false,boots,false);pool(v,false,book,false);pool(v,true,boots,v>=10&&v<=35);pool(v,true,book,v>=10&&v<=35);}
 auto e=make_shared<LivingEntity>();
 int counts[2]={};for(int enchant=1;enchant<=2;++enchant){Level w;lake(w);FrostWalkerEnchantment::freezeNearby(e,&w,0,64,0,enchant);counts[enchant-1]=frozen(w);assert(counts[enchant-1]>0);for(auto b:w.blocks)if(b.second==212){auto[x,y,z]=b.first;double dx=x+.5-e->x,dz=z+.5-e->z;assert(y==63&&dx*dx+.25+dz*dz<=(2+enchant)*(2+enchant));}assert(w.scheduled.size()==(unsigned)counts[enchant-1]);for(auto s:w.scheduled)assert(get<4>(s)>=60&&get<4>(s)<=120);}
 assert(counts[1]>counts[0]);
 // Flowing water, solid cover, other blocks and occupied placement space do not freeze.
 for(int failure=0;failure<9;++failure){Level w;lake(w);if(failure==0)w.isClientSide=true;if(failure==1)e->onGround=false;if(failure==2)w.loaded=false;if(failure==3)w.allowPlace=false;if(failure==4)w.failWrites=true;
 if(failure>=5){auto original=w.blocks;for(auto b:original){if(failure==5)w.data[b.first]=1;if(failure==6){auto[x,y,z]=b.first;w.blocks[{x,y+1,z}]=1;}if(failure==7)w.blocks[b.first]=11;if(failure==8)w.blocks[b.first]=1;}}
 FrostWalkerEnchantment::freezeNearby(e,&w,0,64,0,2);assert(frozen(w)==0&&w.scheduled.empty());e->onGround=true;}
 // Height limits, zero levels, negative coordinates and floor-based movement.
 for(int y:{0,257}){Level w;lake(w,y-1);FrostWalkerEnchantment::freezeNearby(e,&w,0,y,0,2);assert(frozen(w)==0);}
 {Level w;lake(w);FrostWalkerEnchantment::freezeNearby(e,&w,0,64,0,0);assert(frozen(w)==0);}
 {Level w;lake(w);e->x=-.1;e->z=-.1;FrostWalkerEnchantment::freezeNearby(e,&w,-1,64,-1,2);assert(frozen(w)>0);e->x=e->z=.5;}
 {Level w;lake(w,255);e->box.y0=256;FrostWalkerEnchantment::freezeNearby(e,&w,0,256,0,2);assert(frozen(w)>0);e->box.y0=64;}
 // Movement hook uses boots, runs once per cell change, and never runs on clients.
 {Level w;lake(w);e->level=&w;e->equipment[0]=boots;e->frostPositionValid=false;EnchantmentHelper::frostCalls=0;e->updateFrostWalking();assert(frozen(w)==0);e->equipment[1]=boots;e->x=1.1;e->updateFrostWalking();assert(frozen(w)>0);int n=EnchantmentHelper::frostCalls;e->x=1.9;e->updateFrostWalking();assert(EnchantmentHelper::frostCalls==n);e->x=2.1;e->updateFrostWalking();assert(EnchantmentHelper::frostCalls==n+1);w.isClientSide=true;e->x=3.1;e->updateFrostWalking();assert(EnchantmentHelper::frostCalls==n+1);e->x=e->z=.5;}
 FrostedIceTile ice(212);Tile::lightBlock[212]=3;IconRegister icons;ice.registerIcons(&icons);assert(!ice.isSilkTouchable()&&ice.cloneTileId(nullptr,0,0,0)==0&&ice.getResourceCount(nullptr)==0);
 for(int i=0;i<4;++i)assert(ice.getTexture(0,i)->name==L"frosted_ice_"+to_wstring(i));assert(ice.getTexture(0,99)==ice.getTexture(0,3));assert(ice.getTexture(0,-1)==ice.getTexture(0,0));
 // Every stage ages once in light and finally becomes source water, without drops.
 {Level w;Random r;w.blocks[{0,63,0}]=212;for(int i=0;i<3;++i){ice.tick(&w,0,63,0,&r);assert(w.getTile(0,63,0)==212&&w.getData(0,63,0)==i+1);}ice.tick(&w,0,63,0,&r);assert(w.getTile(0,63,0)==9);for(auto s:w.scheduled)assert(get<4>(s)>=20&&get<4>(s)<=40);}
 // Darkness retains the ice and reschedules; daylight resumes normal aging.
 {Level w;Random r;w.blocks[{0,63,0}]=212;w.brightness=0;ice.tick(&w,0,63,0,&r);assert(w.getData(0,63,0)==0&&w.scheduled.size()==1);w.brightness=15;ice.tick(&w,0,63,0,&r);assert(w.getData(0,63,0)==1);}
 // A dense patch requires the 1-in-3 roll; an edge can age without it.
 {Level w;Random r;r.choice=1;w.blocks[{0,63,0}]=212;for(int face=0;face<6;++face)w.blocks[{Facing::STEP_X[face],63+Facing::STEP_Y[face],Facing::STEP_Z[face]}]=212;ice.tick(&w,0,63,0,&r);assert(w.getData(0,63,0)==0);w.blocks[{1,63,0}]=0;w.blocks[{-1,63,0}]=0;w.blocks[{0,64,0}]=0;ice.tick(&w,0,63,0,&r);assert(w.getData(0,63,0)==1);}
 // Melting ages all six immediate neighbors, but does not recursively age beyond them.
 {Level w;Random r;w.blocks[{0,63,0}]=212;w.data[{0,63,0}]=3;for(int face=0;face<6;++face)w.blocks[{Facing::STEP_X[face],63+Facing::STEP_Y[face],Facing::STEP_Z[face]}]=212;w.blocks[{2,63,0}]=212;ice.tick(&w,0,63,0,&r);assert(w.getTile(0,63,0)==9);for(int face=0;face<6;++face)assert(w.getData(Facing::STEP_X[face],63+Facing::STEP_Y[face],Facing::STEP_Z[face])==1);assert(w.getData(2,63,0)==0);}
 // Neighbor loss, Nether evaporation, failure and client authority guards.
 {Level w;w.blocks[{0,63,0}]=212;ice.neighborChanged(&w,0,63,0,1);assert(w.getTile(0,63,0)==212);ice.neighborChanged(&w,0,63,0,212);assert(w.getTile(0,63,0)==9);}
 {Level w;Random r;w.blocks[{0,63,0}]=212;w.data[{0,63,0}]=3;w.dim.ultraWarm=true;ice.tick(&w,0,63,0,&r);assert(w.getTile(0,63,0)==0);}
 {Level w;Random r;w.blocks[{0,63,0}]=212;w.isClientSide=true;ice.tick(&w,0,63,0,&r);ice.neighborChanged(&w,0,63,0,212);assert(w.writes==0&&w.scheduled.empty()&&!ice.shouldTileTick(&w,0,63,0));}
 {Level w;Random r;w.blocks[{0,63,0}]=212;w.failWrites=true;ice.tick(&w,0,63,0,&r);assert(w.getData(0,63,0)==0&&w.scheduled.empty());}
}
'''
def main():
 with tempfile.TemporaryDirectory() as tmp:
  f=Path(tmp);(f/'stdafx.h').write_text(STUB)
  for name in ['Enchantment.h','IceTile.h','IconRegister.h','Facing.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h','net.minecraft.world.level.dimension.h','net.minecraft.world.entity.h','net.minecraft.world.phys.h']:(f/name).write_text('#include "stdafx.h"\n')
  for name in ['FrostWalkerEnchantment.cpp','FrostWalkerEnchantment.h','FrostedIceTile.cpp','FrostedIceTile.h','WaterWalkerEnchantment.cpp','WaterWalkerEnchantment.h']:(f/name).write_bytes((W/name).read_bytes())
  bodies='\n'.join(method(file,sig) for file,sig in [('Enchantment.cpp','bool Enchantment::isCompatibleWith'),('EnchantmentHelper.cpp','unordered_map<int, EnchantmentInstance *> *EnchantmentHelper::getAvailableEnchantmentResults'),('LivingEntity.cpp','void LivingEntity::updateFrostWalking'),('IceTile.cpp','int IceTile::getResourceCount')])
  compile_and_run(f,'frost_walker_production','#include "stdafx.h"\n#include "FrostWalkerEnchantment.h"\n'+bodies+'\n'+TEST)
 assert 'frostWalker = new FrostWalkerEnchantment(9, FREQ_RARE)' in (W/'Enchantment.cpp').read_text()
 assert 'updateFrostWalking();' in method('LivingEntity.cpp','void LivingEntity::tick()')
 assert 'frostPositionValid = false;' in method('LivingEntity.cpp','void LivingEntity::_init()')
 print('Frost Walker registration, movement and treasure checks: passed')

def verify_atlas():
 import hashlib,json,struct,zlib,re
 def png(path):
  b=path.read_bytes();assert b[:8]==b'\x89PNG\r\n\x1a\n';offset=8;data=b''
  while offset<len(b):
   size=struct.unpack('>I',b[offset:offset+4])[0];kind=b[offset+4:offset+8];chunk=b[offset+8:offset+8+size];offset+=12+size
   if kind==b'IHDR':width,height,depth,color,compression,flt,interlace=struct.unpack('>IIBBBBB',chunk);assert(depth,color,interlace)==(8,6,0)
   if kind==b'IDAT':data+=chunk
  raw=zlib.decompress(data);stride=width*4;out=bytearray();prev=bytearray(stride);offset=0
  def paeth(a,b,c):
   p=a+b-c;dist=[abs(p-a),abs(p-b),abs(p-c)];return[a,b,c][dist.index(min(dist))]
  for y in range(height):
   mode=raw[offset];row=bytearray(raw[offset+1:offset+1+stride]);offset+=stride+1
   for x in range(stride):
    left=row[x-4]if x>=4 else 0;up=prev[x];corner=prev[x-4]if x>=4 else 0
    pred=[0,left,up,(left+up)//2,paeth(left,up,corner)][mode];row[x]=(row[x]+pred)%256
   out+=row;prev=row
  return width,height,out
 records=json.loads((ROOT/'tests/frost_atlas_hashes.json').read_text());base=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res'
 for fn,record in records.items():
  width,height,pixels=png(base/fn);s=width//16;masked=bytearray(pixels)
  for i in range(4):
   crop=b''.join(pixels[(y*width+(9+i)*s)*4:(y*width+(10+i)*s)*4]for y in range(25*s,26*s))
   assert hashlib.sha256(crop).hexdigest()==record['stages_sha256'][i]
   if fn=='terrain.png':assert crop==png(base/('textures/blocks/frosted_ice_'+str(i)+'.png'))[2]
   for y in range(25*s,26*s):masked[(y*width+(9+i)*s)*4:(y*width+(10+i)*s)*4]=b'\0'*(s*4)
  assert hashlib.sha256(masked).hexdigest()==record['outside_sha256'],fn
 atlas=(ROOT/'Minecraft.Client/PreStitchedTextureMap.cpp').read_text()
 for i in range(4):assert f'ADD_ICON(25, {9+i}, L"frosted_ice_{i}")'in atlas
 assert 'setDescriptionId(IDS_TILE_FROSTED_ICE)->sendTileData(3)'in(W/'Tile.cpp').read_text()
 print('Frosted ice sprites/mips and preservation of existing atlas pixels: passed')

if __name__=='__main__':
 main()
 verify_atlas()
