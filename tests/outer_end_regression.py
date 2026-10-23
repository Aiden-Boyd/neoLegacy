"""Compile the production End terrain/noise and decoration methods against fixtures.

No GPU, real save-file or multiplayer acceptance is implied by these tests.
"""
from pathlib import Path
import importlib.util
import shutil
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def method(source,signature):
 start=source.index(signature);opening=source.index('{',start);depth=1;end=opening+1
 while depth:
  depth+=(source[end]=='{')-(source[end]=='}');end+=1
 return source[start:end]+'\n'

COMMON=r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
using namespace std;
#define byte unsigned char
#define MemSect(x)
using LARGE_INTEGER=int64_t;
inline void QueryPerformanceCounter(int64_t *p){*p=0;}
template<class T>struct Array {T *data=nullptr;unsigned int length=0;Array()=default;explicit Array(int n):data(new T[n]()),length(n){}T& operator[](int i){assert(i>=0&&unsigned(i)<length);return data[i];}};
using byteArray=Array<unsigned char>;using doubleArray=Array<double>;struct BiomeArray{};
struct Tile {enum {stone_Id=1,endStone_Id=121,chorusPlant_Id=199,chorusFlower_Id=200,UPDATE_CLIENTS=2};};
struct Level {
 static const int genDepth=128,genDepthBits=7,genDepthBitsPlusFour=11,maxBuildHeight=256;
 bool isClientSide=false,flat=false,bounded=false;int minX=0,minZ=0;
 map<tuple<int,int,int>,int> blocks,data;vector<tuple<int,int,int,int,int>> writes;
 void check(int x,int y,int z){assert(y>=0&&y<maxBuildHeight);if(bounded){assert(x>=minX&&x<minX+32&&z>=minZ&&z<minZ+32);}}
 int getTile(int x,int y,int z){check(x,y,z);auto it=blocks.find({x,y,z});return it==blocks.end()?(flat&&y<=60?Tile::endStone_Id:0):it->second;}
 bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}
 bool setTileAndData(int x,int y,int z,int id,int aux,int flags){check(x,y,z);int old=getTile(x,y,z);assert(old==0||(old==Tile::chorusPlant_Id&&id==Tile::chorusFlower_Id));assert(flags==Tile::UPDATE_CLIENTS);blocks[{x,y,z}]=id;data[{x,y,z}]=aux;writes.push_back({x,y,z,id,aux});return true;}
 int getHeightmap(int x,int z){for(int y=maxBuildHeight-1;y>=0;--y)if(getTile(x,y,z)!=0)return y+1;return 0;}
};
struct Facing {enum {DOWN,UP,NORTH,SOUTH,WEST,EAST};inline static int STEP_X[6]={0,0,0,0,-1,1},STEP_Y[6]={-1,1,0,0,0,0},STEP_Z[6]={0,0,-1,1,0,0};static int getOpposite(int f){return f^1;}};
#include "Random.h"
// Match Windows headers so new code cannot accidentally depend on unguarded std::min/max.
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
'''

GENERATOR=r'''
class CLASSNAME {
public:
 static const int CHUNK_WIDTH=8,CHUNK_HEIGHT=4;int m_XZSize=0;Level *level;
 Random *random,*pprandom;PerlinNoise *lperlinNoise1,*lperlinNoise2,*perlinNoise1,*scaleNoise,*depthNoise;SimplexNoise *islandNoise=nullptr;
 CLASSNAME(Level*,int64_t);~CLASSNAME();
 doubleArray getHeights(doubleArray,int,int,int,int,int,int);
 void prepareHeights(int,int,byteArray,BiomeArray);
};
'''

TERRAIN_TEST=r'''
template<class G>uint64_t chunkHash(G &g,int x,int z,int *land=nullptr){
 byteArray blocks(Level::genDepth*256);g.prepareHeights(x,z,blocks,BiomeArray{});
 uint64_t hash=1469598103934665603ULL;int count=0;
 for(unsigned i=0;i<blocks.length;++i){hash=(hash^blocks[i])*1099511628211ULL;if(blocks[i])++count;}
 delete[] blocks.data;if(land)*land=count;return hash;
}
int main(){
 Level world;
 for(int64_t seed:{0LL,1LL,987654321LL,-12345LL}){
  TheEndLevelRandomLevelSource current(&world,seed);LegacyEndSource old(&world,seed);
  // Compare actual generated block buffers throughout the entire original 18x18-chunk End.
  for(int x=-9;x<9;++x)for(int z=-9;z<9;++z)assert(chunkHash(current,x,z)==chunkHash(old,x,z));
  int central=0,gap=0;chunkHash(current,0,0,&central);chunkHash(current,32,0,&gap);assert(central>0&&gap==0);
  int islands=0,voids=0,quadrants=0;
  for(int x=-112;x<=112;x+=16)for(int z=-112;z<=112;z+=16){
   if(int64_t(x)*x+int64_t(z)*z<72*72)continue;
   int land=0;chunkHash(current,x,z,&land);
   if(land){++islands;quadrants|=1<<((x<0?1:0)+(z<0?2:0));}else ++voids;
  }
  assert(islands>0&&voids>0&&quadrants==15);
  auto first=chunkHash(current,96,-80);chunkHash(current,-96,80);assert(first==chunkHash(current,96,-80)); // order independence
  for(int x:{-97,-1,0,96})for(int z:{-80,-1,0,80}){
   doubleArray left=current.getHeights({},x*2,0,z*2,3,33,3),right=current.getHeights({},(x+1)*2,0,z*2,3,33,3);
   for(int zz=0;zz<3;++zz)for(int y=0;y<33;++y)assert(std::abs(left[(2*3+zz)*33+y]-right[zz*33+y])<1e-6);
   delete[] left.data;delete[] right.data;
  }
  cout<<"seed "<<seed<<": central blocks preserved, "<<islands<<" sampled island chunks and "<<voids<<" void chunks\n";
 }
 // Samples on all central lattice points retain the old radial field exactly.
 Random rng(7);SimplexNoise noise(&rng);
 for(int x=-18;x<=18;++x)for(int z=-18;z<=18;++z)assert(EndIslandTerrain::height(&noise,x,z)==EndIslandTerrain::mainIslandHeight(x,z));
 assert(!EndIslandTerrain::isOuterChunk(64,0)&&EndIslandTerrain::isOuterChunk(65,0));
}
'''

FEATURE_TEST=r'''
struct ChorusPlantTile {int id=199;bool canSurvive(Level*,int,int,int);};
struct ChorusFlowerTile {bool canSurvive(Level*,int,int,int);};
SUPPORT_METHODS
int main(){
 for(int seed=0;seed<64;++seed){
  Random random(seed);Level w;w.flat=true;w.bounded=true;
  assert(EndOuterFeatures::growChorus(&w,&random,16,61,16));int flowers=0;
  ChorusPlantTile plant;ChorusFlowerTile flower;
  for(auto entry:w.blocks){auto [x,y,z]=entry.first;int id=entry.second;
   assert(x>8&&x<24&&z>8&&z<24&&y>=61&&y<90);
   if(id==199)assert(plant.canSurvive(&w,x,y,z));
   else {assert(id==200&&w.data[entry.first]==5&&flower.canSurvive(&w,x,y,z));++flowers;}
  }assert(flowers>0);
 }
 // Obstructed growth never replaces terrain or player blocks.
 Level blocked;blocked.flat=true;blocked.blocks[{16,64,16}]=49;Random a(0);
 assert(EndOuterFeatures::growChorus(&blocked,&a,16,61,16));assert(blocked.getTile(16,64,16)==49);
 Level client;client.flat=true;client.isClientSide=true;assert(!EndOuterFeatures::growChorus(&client,&a,0,61,0));EndOuterFeatures::placeSmallIsland(&client,&a,0,60,0);assert(client.writes.empty());
 Level invalid;assert(!EndOuterFeatures::growChorus(&invalid,&a,0,1,0));
 int smallIslands=0,trees=0;
 for(int seed=0;seed<80;++seed){
  Level w;w.bounded=true;w.minX=-1280;w.minZ=1280;Random random(seed);
  EndOuterFeatures::decorate(&w,&random,-80,80,-50);if(!w.writes.empty())++smallIslands;
  for(auto write:w.writes){auto [x,y,z,id,data]=write;assert(id==121&&y>=40&&y<=70);}
  Level big;big.flat=true;big.bounded=true;big.minX=1280;big.minZ=-1280;Random treeRandom(seed);
  EndOuterFeatures::decorate(&big,&treeRandom,80,-80,80);if(!big.writes.empty())++trees;
  Level repeat;repeat.flat=true;repeat.bounded=true;repeat.minX=1280;repeat.minZ=-1280;Random same(seed);
  EndOuterFeatures::decorate(&repeat,&same,80,-80,80);assert(big.writes==repeat.writes); // seeded decoration
 }
 assert(smallIslands>0&&trees>0);
 Level center;center.flat=true;Random r(0);EndOuterFeatures::decorate(&center,&r,0,0,80);assert(center.writes.empty());
 cout<<"chorus support, bounds, blocked growth and deterministic outer decoration passed\n";
}
'''
BOUNDS=r'''
struct TheEndDimension {int getXZSize();};
struct LevelRenderer {
 static const int CHUNK_XZSIZE=16,CHUNK_SIZE=16,CHUNK_Y_COUNT=16,PLAYER_VIEW_DISTANCE=18;
 static const int MAX_LEVEL_RENDER_SIZE[3],DIMENSION_OFFSETS[3];
 static int getDimensionIndexFromId(int);
 static int getGlobalIndexForChunk(int,int,int,int);
 static int getGlobalChunkCount();
};
BOUNDS_METHODS
int main(){
 TheEndDimension end;
 assert(end.getXZSize()==256&&END_LEVEL_MIN_WIDTH==END_LEVEL_MAX_WIDTH);
 assert(LevelRenderer::MAX_LEVEL_RENDER_SIZE[2]==end.getXZSize());
 vector<bool> indices(LevelRenderer::getGlobalChunkCount(),false);
 for(int x=-128;x<128;++x)for(int z=-128;z<128;++z)for(int y=0;y<256;y+=16){
  int index=LevelRenderer::getGlobalIndexForChunk(x*16,y,z*16,1);
  assert(index>=LevelRenderer::DIMENSION_OFFSETS[2]&&index<int(indices.size())&&!indices[index]);indices[index]=true;
 }
 assert(LevelRenderer::getGlobalIndexForChunk(-2064,0,0,1)==-1);
 assert(LevelRenderer::getGlobalIndexForChunk(2048,0,0,1)==-1);
 assert(LevelRenderer::getGlobalIndexForChunk(0,256,0,1)==-1);
 cout<<"End dimension, renderer indices and finite boundary agree\n";
}
'''

def main():
 spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 source=(ROOT/'Minecraft.World/TheEndLevelRandomLevelSource.cpp').read_text(encoding='utf-8')
 pieces=''.join(method(source,s) for s in ['TheEndLevelRandomLevelSource::TheEndLevelRandomLevelSource(', 'TheEndLevelRandomLevelSource::~TheEndLevelRandomLevelSource(', 'void TheEndLevelRandomLevelSource::prepareHeights(', 'doubleArray TheEndLevelRandomLevelSource::getHeights('])
 # Independent control: the exact radial calculation from the pre-expansion source.
 legacy_density='''float xd = ((xx + x) - 0) / 1.0f;
            float zd = ((zz + z) - 0) / 1.0f;
            float doffs = 100 - sqrt(xd * xd + zd * zd) * 8;
            if (doffs > 80) doffs = 80;
            if (doffs < -100) doffs = -100;'''
 old=pieces.replace('TheEndLevelRandomLevelSource','LegacyEndSource').replace('float doffs = EndIslandTerrain::height(islandNoise,xx+x,zz+z);',legacy_density).replace('islandNoise = new SimplexNoise(random);','islandNoise = nullptr;')
 with tempfile.TemporaryDirectory() as temp:
  folder=Path(temp);(folder/'stdafx.h').write_text(COMMON,encoding='utf-8')
  for name in ['System.h','Facing.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h']:(folder/name).write_text('#include "stdafx.h"\n',encoding='utf-8')
  (folder/'Biome.h').write_text('#pragma once\n',encoding='utf-8')
  bounds=(ROOT/'Minecraft.World/ChunkSource.h').read_text(encoding='utf-8').split('class ChunkSource')[0]
  (folder/'EndBounds.h').write_text('#define _LARGE_WORLDS\n'+bounds,encoding='utf-8')
  (folder/'Mth.h').write_text('#pragma once\nstruct Mth {static int64_t lfloor(double v){return int64_t(std::floor(v));}};\n',encoding='utf-8')
  for name in ['Random.h','Random.cpp','Synth.h','ImprovedNoise.h','ImprovedNoise.cpp','PerlinNoise.h','PerlinNoise.cpp','SimplexNoise.h','SimplexNoise.cpp','EndIslandTerrain.h','EndOuterFeatures.h','EndOuterFeatures.cpp']:shutil.copyfile(ROOT/'Minecraft.World'/name,folder/name)
  includes='#include "EndBounds.h"\n#include "Random.cpp"\n#include "ImprovedNoise.cpp"\n#include "PerlinNoise.cpp"\n#include "SimplexNoise.cpp"\n#include "EndIslandTerrain.h"\n'
  base.compile_and_run(folder,'outer_end_terrain',includes+GENERATOR.replace('CLASSNAME','TheEndLevelRandomLevelSource')+pieces+GENERATOR.replace('CLASSNAME','LegacyEndSource')+old+TERRAIN_TEST)
  tiles=(ROOT/'Minecraft.World/EndPlantTiles.cpp').read_text(encoding='utf-8')
  support=method(tiles,'bool ChorusPlantTile::canSurvive(')+method(tiles,'bool ChorusFlowerTile::canSurvive(')
  base.compile_and_run(folder,'outer_end_features',includes+'#include "EndOuterFeatures.cpp"\n'+FEATURE_TEST.replace('SUPPORT_METHODS',support))
  renderer=(ROOT/'Minecraft.Client/LevelRenderer.cpp').read_text(encoding='utf-8')
  constants=renderer[renderer.index('const int overworldSize ='):renderer.index('#else',renderer.index('const int overworldSize ='))]
  methods=method((ROOT/'Minecraft.World/TheEndDimension.cpp').read_text(encoding='utf-8'),'int TheEndDimension::getXZSize(')+constants
  for name in ['int LevelRenderer::getDimensionIndexFromId(', 'int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, int dimensionId)', 'int LevelRenderer::getGlobalChunkCount(']:methods+=method(renderer,name)
  base.compile_and_run(folder,'outer_end_bounds','#include "stdafx.h"\n#include "EndBounds.h"\n'+BOUNDS.replace('BOUNDS_METHODS',methods))

if __name__=='__main__':main()
