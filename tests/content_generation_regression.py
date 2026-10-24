"""Compile real ore placement plus the appended magma population block."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
STUB=r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <map>
#include <tuple>
using namespace std;
#define PIXBeginNamedEvent(...)
#define PIXEndNamedEvent(...)
const double PI=3.141592653589793;
struct Mth{static double sin(double d){return std::sin(d);}static double cos(double d){return std::cos(d);}static int floor(double d){return (int)std::floor(d);}};
struct Random {uint64_t state;Random(int64_t s):state(s){}uint32_t next(){state=state*6364136223846793005ULL+1442695040888963407ULL;return uint32_t(state>>32);}int nextInt(int n){return next()%n;}float nextFloat(){return (next()>>8)*(1.0f/16777216);}double nextDouble(){return next()*(1.0/4294967296.0);}};
struct Tile{enum{magma_Id=213,netherRack_Id=87,stone_Id=1,UPDATE_CLIENTS=2,UPDATE_INVISIBLE_NO_LIGHT=12};};
struct LevelGenerationOptions{bool checkIntersects(int,int,int,int,int,int){return false;}};
struct App{LevelGenerationOptions*getLevelGenerationOptions(){return nullptr;}}app;
struct Level{int64_t seed;bool loaded=true;int writes=0;map<tuple<int,int,int>,int>blocks;Level(int64_t s):seed(s){}int64_t getSeed(){return seed;}int getSeaLevel(){return 32;}int getHeightmap(int,int){return 128;}bool hasChunkAt(int,int,int){return loaded;}int getTile(int x,int y,int z){return blocks[{x,y,z}];}void setTileAndData(int x,int y,int z,int id,int data,int flags){assert(data==0);assert(flags==(id==213?2:12));assert(blocks[make_tuple(x,y,z)]==87);blocks[{x,y,z}]=id;++writes;}};
struct Feature{};
'''
TEST=r'''
void fill(Level&w,int xo,int zo){for(int x=xo-8;x<=xo+40;++x)for(int z=zo-8;z<=zo+40;++z)for(int y=20;y<=42;++y)w.blocks[{x,y,z}]=((x+y+z)%11==0?1:87);}
int main(){for(int64_t seed:{0LL,1LL,42LL,-900LL})for(int chunk:{0,-2}){Level a(seed),b(seed);fill(a,chunk*16,chunk*16);b.blocks=a.blocks;auto original=a.blocks;populate(&a,chunk,chunk);populate(&b,chunk,chunk);assert(a.writes>0&&a.blocks==b.blocks);for(auto entry:original)if(entry.second!=87)assert(a.blocks[entry.first]==entry.second);for(auto entry:a.blocks)if(entry.second==213){auto[x,y,z]=entry.first;assert(y>=20&&y<=42);}Level unloaded(seed);fill(unloaded,0,0);unloaded.loaded=false;populate(&unloaded,0,0);assert(unloaded.writes==0);}
 // Existing ordinary ores retain their previous update flags and target restriction.
 Level ore(42);fill(ore,0,0);Random r(2);OreFeature feature(153,0,13,87);feature.place(&ore,&r,0,30,0);assert(ore.writes>0);
}
'''
def verify_generation():
 source=(W/'HellRandomLevelSource.cpp').read_text();start=source.index('    Random magmaRandom(');end=source.index('\n\tHeavyTile::instaFall = false;',start)
 body='void populate(Level*level,int xt,int zt){int xo=xt*16,zo=zt*16;\n'+source[start:end]+'\n}\n'
 with tempfile.TemporaryDirectory()as tmp:
  f=Path(tmp);(f/'stdafx.h').write_text(STUB)
  for h in ['Feature.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h']:(f/h).write_text('#include "stdafx.h"\n')
  for h in ['OreFeature.h','OreFeature.cpp']:(f/h).write_bytes((W/h).read_bytes())
  compile_and_run(f,'magma_population','#include "stdafx.h"\n#include "OreFeature.cpp"\n'+body+TEST)
if __name__=='__main__':verify_generation()
