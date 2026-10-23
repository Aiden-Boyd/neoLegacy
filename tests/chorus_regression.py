"""Compile production chorus growth, rod geometry and fruit use against engine fixtures.

These checks do not establish visual, network or CU59 parity; see CHORUS_TESTS.md.
"""
import importlib.util
from pathlib import Path
import shutil
import tempfile

ROOT=Path(__file__).resolve().parents[1]
STUBS=r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
enum {eParticleType_endRod,eSoundType_MOB_ENDERMEN_PORTAL};
struct Icon {wstring name;float getU0(bool){return 0;}float getU1(bool){return 1;}float getV0(bool){return 0;}float getV1(bool){return 1;}};
struct IconRegister {map<wstring,Icon> icons;Icon *registerIcon(wstring n){return &(icons[n]={n});}};
struct Material {bool solid=false;bool blocksMotion(){return solid;}static Material *plant,*decoration;};
inline Material airMaterial,solidMaterial{true};
inline Material *Material::plant=&airMaterial,*Material::decoration=&airMaterial;
struct Random {deque<int> values;int nextInt(int n){if(values.empty())return n==16?8:0;int v=values.front();values.pop_front();return v%n;}double nextDouble(){return .5;}float nextFloat(){return .5f;}double nextGaussian(){return 0;}};
struct AABB {double x0,y0,z0,x1,y1,z1;static AABB *newTemp(double a,double b,double c,double d,double e,double f){static deque<AABB> boxes;boxes.push_back({a,b,c,d,e,f});return &boxes.back();}bool intersects(AABB *b){return x1>b->x0&&x0<b->x1&&y1>b->y0&&y0<b->y1&&z1>b->z0&&z0<b->z1;}};
struct Entity {};struct TileEntity {};using AABBList=vector<AABB*>;
struct ItemInstance {int id=432,count=1;ItemInstance()=default;ItemInstance(int id,int n,int):id(id),count(n){}};
struct Level;
struct Player:Entity {int chorusFruitCooldown=0,uses=0,teleports=0;double x=.5,y=70,z=.5;float fallDistance=10;struct {bool instabuild=false;} abilities;shared_ptr<Entity> riding;AABB *bb=AABB::newTemp(.2,70,.2,.8,71.8,.8);void startUsingItem(shared_ptr<ItemInstance>,int){++uses;}void ride(shared_ptr<Entity> e){riding=e;}virtual void teleportTo(double a,double b,double c){++teleports;x=a;y=b;z=c;}};
struct LevelSource {map<tuple<int,int,int>,int> blocks,data;int getTile(int x,int y,int z){return blocks[{x,y,z}];}int getData(int x,int y,int z){return data[{x,y,z}];}float getBrightness(int,int,int){return 1;}};
struct Level:LevelSource {static const int maxBuildHeight=256;bool isClientSide=false,loaded=true,collision=false,liquid=false;int destroyed=0,scheduled=0,drops=0;Random rng,*random=&rng;AABBList cubes;
 bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}
 bool setTileAndData(int x,int y,int z,int id,int aux,int){blocks[{x,y,z}]=id;data[{x,y,z}]=aux;return true;}
 void setData(int x,int y,int z,int aux,int){data[{x,y,z}]=aux;}
 void destroyTile(int x,int y,int z,bool){blocks[{x,y,z}]=0;++destroyed;}
 void addToTickNextTick(int,int,int,int,int){++scheduled;}
 void addParticle(int,double,double,double,double,double,double){}
 bool hasChunkAt(int,int,int){return loaded;}bool hasChunksAt(int,int,int,int,int,int){return loaded;}
 Material *getMaterial(int,int y,int){return y<=64?&solidMaterial:&airMaterial;}
 AABBList *getCubes(shared_ptr<Entity>,AABB *box){cubes.clear();if(collision||box->y0<65)cubes.push_back(nullptr);return &cubes;}
 bool containsAnyLiquid(AABB*){return liquid;}
 void playSound(double,double,double,int,float,float){}void playEntitySound(shared_ptr<Entity>,int,float,float){}
};
struct Facing {enum {DOWN,UP,NORTH,SOUTH,WEST,EAST};inline static int STEP_X[6]={0,0,0,0,-1,1},STEP_Y[6]={-1,1,0,0,0,0},STEP_Z[6]={0,0,-1,1,0,0};static int getOpposite(int f){return f^1;}};
struct Item {enum {chorusFruit_Id=432};};
struct FoodItem:Item {FoodItem(int,int,float,bool){}virtual ~FoodItem()=default;void setCanAlwaysEat(){}int getUseDuration(shared_ptr<ItemInstance>){return 32;}virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> i,Level*,shared_ptr<Player>){return i;}virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> i,Level*,shared_ptr<Player>){--i->count;return i;}};
struct Tile {enum {SHAPE_END_PLANT=41,endStone_Id=121,chorusPlant_Id=199,chorusFlower_Id=200,UPDATE_ALL=3,UPDATE_CLIENTS=2};int id;Icon *icon=nullptr;float x0=0,y0=0,z0=0,x1=1,y1=1,z1=1;Tile(int i,Material*,bool):id(i){}virtual ~Tile()=default;
 void setLightBlock(int){}void setTicking(bool){}void setShape(float a,float b,float c,float d,float e,float f){x0=a;y0=b;z0=c;x1=d;y1=e;z1=f;}
 virtual bool isSolidRender(bool=false){return false;}virtual bool isCubeShaped(){return false;}virtual bool isPathfindable(LevelSource*,int,int,int){return false;}virtual int getRenderShape(){return 0;}virtual bool isSilkTouchable(){return false;}
 virtual void updateShape(LevelSource*,int,int,int,int=-1,shared_ptr<TileEntity> ={}){}virtual void updateDefaultShape(){}
 virtual AABB *getAABB(Level*,int x,int y,int z){return AABB::newTemp(x+x0,y+y0,z+z0,x+x1,y+y1,z+z1);}virtual AABB *getTileAABB(Level *l,int x,int y,int z){return getAABB(l,x,y,z);}
 virtual void addAABBs(Level*,int,int,int,AABB*,AABBList*,shared_ptr<Entity>){}
 virtual int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int){return 0;}virtual void animateTick(Level*,int,int,int,Random*){}
 virtual int getSpawnResourcesAuxValue(int){return 0;}virtual bool canSurvive(Level*,int,int,int){return true;}virtual bool mayPlace(Level*,int,int,int){return true;}
 virtual void neighborChanged(Level*,int,int,int,int){}virtual void tick(Level*,int,int,int,Random*){}virtual int getResource(int,Random*,int){return id;}virtual int getResourceCount(Random*){return 1;}
 virtual void registerIcons(IconRegister*){}virtual Icon *getTexture(int,int){return icon;}virtual void playerDestroy(Level*,shared_ptr<Player>,int,int,int,int){}
 void popResource(Level *l,int,int,int,shared_ptr<ItemInstance>){++l->drops;}
};
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
'''
TEST=r'''
#include "EndPlantTiles.cpp"
#include "ChorusFruitItem.cpp"
int main(){
 EndRodTile rod(198);ChorusPlantTile plant(199);ChorusFlowerTile flower(200);EndPlantPart parts[EndPlantTile::MAX_PARTS];Level w;
 for(int face=0;face<6;++face){
  assert(rod.getPlacedOnFaceDataValue(&w,0,0,0,face,0,0,0,0)==face);
  int x=-Facing::STEP_X[face],y=-Facing::STEP_Y[face],z=-Facing::STEP_Z[face];
  w.blocks[{x,y,z}]=198;w.data[{x,y,z}]=face;
  assert(rod.getPlacedOnFaceDataValue(&w,0,0,0,face,0,0,0,0)==(face^1));w.blocks.clear();
  assert(rod.getParts(nullptr,0,0,0,face,parts)==2);
  float x0=8,y0=16,z0=8;rotateEndPlantPoint(face,x0,y0,z0);
  assert(x0==8+8*Facing::STEP_X[face]&&y0==8+8*Facing::STEP_Y[face]&&z0==8+8*Facing::STEP_Z[face]);
 }
 assert(!flower.canSurvive(&w,0,1,0));w.blocks[{0,0,0}]=121;assert(flower.canSurvive(&w,0,1,0));
 w.blocks[{0,1,0}]=200;flower.tick(&w,0,1,0,&w.rng);
 assert(w.getTile(0,1,0)==199&&w.getTile(0,2,0)==200); // vertical growth
 w.data[{0,2,0}]=5;flower.tick(&w,0,2,0,&w.rng);assert(w.getTile(0,3,0)==0); // dead flower stays dead
 w.blocks.clear();w.data.clear();w.blocks[{0,0,0}]=121;w.blocks[{0,1,0}]=200;w.blocks[{0,3,0}]=1;
 w.rng.values={1,0};flower.tick(&w,0,1,0,&w.rng);assert(w.getTile(0,1,0)==199&&w.getTile(0,1,-1)==200); // branch when upward path blocked
 assert(flower.canSurvive(&w,0,1,-1));w.blocks[{0,0,0}]=0;
 plant.neighborChanged(&w,0,1,0,0);assert(w.scheduled==1);plant.tick(&w,0,1,0,&w.rng);assert(w.destroyed==1);
 flower.tick(&w,0,1,-1,&w.rng);assert(w.destroyed==2); // unsupported branch dies
 assert(flower.getResource(0,&w.rng,0)==0);flower.playerDestroy(&w,{},0,0,0,5);assert(w.drops==1);
 IconRegister icons;flower.registerIcons(&icons);assert(flower.getPartTexture(1,5)->name==L"chorus_flower_dead");assert(flower.getPartTexture(0,5)->name==L"chorus_plant");
 w.blocks.clear();w.blocks[{0,0,0}]=121;w.blocks[{0,1,0}]=199;int partCount=plant.getParts(&w,0,1,0,0,parts);assert(partCount>=6&&partCount<=EndPlantTile::MAX_PARTS);assert(parts[0].facing==Facing::DOWN&&!(parts[0].faces&(1u<<Facing::UP)));
 w.isClientSide=true;int destroyed=w.destroyed;flower.tick(&w,9,9,9,&w.rng);assert(w.destroyed==destroyed);
 ChorusFruitItem fruit(176);auto player=make_shared<Player>();auto item=make_shared<ItemInstance>();item->count=3;
 fruit.use(item,&w,player);assert(player->uses==1);fruit.useTimeDepleted(item,&w,player);assert(item->count==2&&player->chorusFruitCooldown==20&&player->teleports==0);
 fruit.use(item,&w,player);fruit.useTimeDepleted(item,&w,player);assert(player->uses==1&&item->count==2); // cooldown
 w.isClientSide=false;player->chorusFruitCooldown=0;player->abilities.instabuild=true;
 fruit.useTimeDepleted(item,&w,player);assert(item->count==2&&player->teleports==1&&player->fallDistance==0);
 auto eyeOffset=make_shared<Player>();eyeOffset->y+=1.62;assert(ChorusFruitItem::teleport(&w,eyeOffset));assert(eyeOffset->teleports==1&&eyeOffset->y==65);
 for(int failure=0;failure<3;++failure){auto p=make_shared<Player>();w.loaded=failure!=0;w.collision=failure==1;w.liquid=failure==2;assert(!ChorusFruitItem::teleport(&w,p));assert(p->teleports==0&&p->y==70);}
}
'''
RENDER_STUBS=r'''
struct SharedConstants {inline static bool TEXTURE_LIGHTING=true;};
struct Tesselator {
 struct Vertex {double x,y,z,u,v;};vector<Vertex> vertices;int light=0;
 static Tesselator *getInstance(){static Tesselator t;return &t;}
 void color(float,float,float){}void normal(float,float,float){}void tex2(int v){light=v;}
 void vertexUV(double x,double y,double z,double u,double v){vertices.push_back({x,y,z,u,v});}
};
struct TileRenderer {
 LevelSource *level;Icon *fixedTexture=nullptr;
 Icon *getTextureOrMissing(Icon *i){assert(i);return i;}
 int getLightColor(Tile*,LevelSource*,int,int,int){return 123;}
 bool tesselateEndPlant(Tile*,int,int,int,int,bool);
};
'''
RENDER_TEST=r'''
int main(){
 Level w;TileRenderer renderer{&w};Icon texture;EndRodTile rod(198);ChorusFlowerTile flower(200);ChorusPlantTile plant(199);IconRegister icons;
 rod.icon=&texture;plant.icon=&texture;flower.registerIcons(&icons);
 for(auto tile:{static_cast<EndPlantTile*>(&rod),static_cast<EndPlantTile*>(&flower),static_cast<EndPlantTile*>(&plant)})for(int data=0;data<6;++data){
  auto t=Tesselator::getInstance();t->vertices.clear();renderer.tesselateEndPlant(tile,0,0,0,data,false);assert(t->light==123);
  assert(!t->vertices.empty()&&t->vertices.size()%4==0);
  for(auto v:t->vertices){assert(v.x>=0&&v.x<=1&&v.y>=0&&v.y<=1&&v.z>=0&&v.z<=1);assert(v.u>=0&&v.u<=1&&v.v>=0&&v.v<=1);}
  EndPlantPart parts[EndPlantTile::MAX_PARTS];int count=tile->getParts(&w,0,0,0,data,parts);size_t i=0;
  for(int part=0;part<count;++part)for(int face=0;face<6;++face)if(parts[part].faces&(1u<<face)){
   float cx=(parts[part].x0+parts[part].x1)/2,cy=(parts[part].y0+parts[part].y1)/2,cz=(parts[part].z0+parts[part].z1)/2;
   rotateEndPlantPoint(parts[part].facing,cx,cy,cz);cx/=16;cy/=16;cz/=16;
   auto a=t->vertices[i],b=t->vertices[i+1],c=t->vertices[i+2];i+=4;
   double nx=(b.y-a.y)*(c.z-a.z)-(b.z-a.z)*(c.y-a.y),ny=(b.z-a.z)*(c.x-a.x)-(b.x-a.x)*(c.z-a.z),nz=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
   assert(nx*nx+ny*ny+nz*nz>0); // every emitted quad has area
   assert(nx*(a.x-cx)+ny*(a.y-cy)+nz*(a.z-cz)>0); // outward winding, including all rod rotations
  }
  t->vertices.clear();renderer.tesselateEndPlant(tile,0,0,0,data,true);assert(!t->vertices.empty());
 }
}
'''
def main():
 spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 with tempfile.TemporaryDirectory() as tmp:
  folder=Path(tmp);(folder/'stdafx.h').write_text(STUBS,encoding='utf-8')
  for name in ['Tile.h','FoodItem.h','Facing.h','IconRegister.h','Item.h','ItemInstance.h','Random.h','SoundTypes.h','Material.h','net.minecraft.world.level.h','net.minecraft.world.phys.h','net.minecraft.world.entity.player.h']:
   (folder/name).write_text('#include "stdafx.h"\n',encoding='utf-8')
  for name in ['EndPlantTiles.h','EndPlantTiles.cpp','EndPlantModels.h','ChorusFruitItem.h','ChorusFruitItem.cpp']:shutil.copyfile(ROOT/'Minecraft.World'/name,folder/name)
  base.compile_and_run(folder,'chorus_regression',TEST)
  source=(ROOT/'Minecraft.Client/TileRenderer.cpp').read_text(encoding='utf-8')
  start=source.index('bool TileRenderer::tesselateEndPlant(');end=source.index('void TileRenderer::renderTile(',start)
  base.compile_and_run(folder,'chorus_geometry','#include "EndPlantTiles.cpp"\n'+RENDER_STUBS+source[start:end]+RENDER_TEST)
 print('Fixture checks passed; in-game acceptance remains required.')
if __name__=='__main__':main()
