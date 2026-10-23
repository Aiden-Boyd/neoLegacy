"""Compile production gateway behavior and persistence against deterministic fixtures."""
from pathlib import Path
import importlib.util
import shutil
import tempfile
ROOT=Path(__file__).resolve().parents[1]
STUBS=r'''
#pragma once
#include <algorithm>
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
enum eINSTANCEOF {eTYPE_TILEENTITY,eTYPE_ENDGATEWAYTILEENTITY,eTYPE_ENDERDRAGON};
enum {eParticleType_ender,END_LEVEL_MIN_WIDTH=256};
struct Random {uint64_t seed;explicit Random(int64_t seed=0):seed(seed){}int nextInt(int n){seed=seed*6364136223846793005ULL+1;return int((seed>>32)%n);}double nextDouble(){return nextInt(10000)/10000.;}float nextFloat(){return float(nextDouble());}};
struct CompoundTag {map<wstring,int64_t> values;int getInt(wstring n){return int(values[n]);}int64_t getLong(wstring n){return values[n];}bool getBoolean(wstring n){return values[n]!=0;}void putInt(wstring n,int v){values[n]=v;}void putLong(wstring n,int64_t v){values[n]=v;}void putBoolean(wstring n,bool v){values[n]=v;}};
struct Material {bool solid=false;bool blocksMotion(){return solid;}static Material *portal;};inline Material solidMaterial{true},portalMaterial;inline Material *Material::portal=&portalMaterial;
struct AABB {double x0,y0,z0,x1,y1,z1;static AABB *newTemp(double a,double b,double c,double d,double e,double f){static AABB box;box={a,b,c,d,e,f};return &box;}};
using AABBList=vector<AABB*>;
struct Level;struct LevelChunk{};class TileEntity;
struct Packet {virtual ~Packet()=default;};
struct TileEntityDataPacket:Packet {static constexpr int TYPE_END_GATEWAY=8;int type;CompoundTag *tag;TileEntityDataPacket(int,int,int,int t,CompoundTag *p):type(t),tag(p){}~TileEntityDataPacket(){delete tag;}};
struct Entity {virtual ~Entity()=default;Level *level=nullptr;bool removed=false;shared_ptr<Entity> riding;weak_ptr<Entity> rider;float bbWidth=.6f,bbHeight=1.8f,fallDistance=10,yRot=0,xRot=0;double x=0,y=0,z=0;int teleports=0;virtual eINSTANCEOF GetType(){return eTYPE_TILEENTITY;}void moveTo(double a,double b,double c,float,float){x=a;y=b;z=c;++teleports;}void remove(){removed=true;}};
struct LivingEntity:Entity {virtual void teleportTo(double a,double b,double c){moveTo(a,b,c,0,0);}};
struct ThrownEnderpearl:Entity {shared_ptr<LivingEntity> owner;shared_ptr<LivingEntity> getOwner(){return owner;}};
struct LevelData {int endGatewaysGenerated=0;virtual int getEndGatewaysGenerated();virtual void setEndGatewaysGenerated(int);};
struct DerivedLevelData:LevelData {LevelData *wrapped;explicit DerivedLevelData(LevelData *p):wrapped(p){}int getEndGatewaysGenerated() override;void setEndGatewaysGenerated(int) override;};
struct Dimension {int id=1;int getXZSize(){return 256;}};
struct Tile {enum {endGateway_Id=209,unbreakable_Id=7,endStone_Id=121,endPortalTile_Id=119,UPDATE_ALL=3,SHAPE_INVISIBLE=-1};inline static Tile *tiles[256]={};int id;Material *material;Tile(int i,Material *m):id(i),material(m){tiles[i]=this;}virtual ~Tile()=default;virtual bool isCubeShaped(){return true;}};
struct TileEntity {Level *level=nullptr;int x=0,y=0,z=0,changes=0;virtual ~TileEntity()=default;virtual eINSTANCEOF GetType(){return eTYPE_TILEENTITY;}virtual void load(CompoundTag *tag){x=tag->getInt(L"x");y=tag->getInt(L"y");z=tag->getInt(L"z");}virtual void save(CompoundTag *tag){tag->putInt(L"x",x);tag->putInt(L"y",y);tag->putInt(L"z",z);}virtual shared_ptr<Packet> getUpdatePacket(){return{};}virtual shared_ptr<TileEntity> clone()=0;void clone(shared_ptr<TileEntity> other){other->level=level;other->x=x;other->y=y;other->z=z;}virtual void tick(){}virtual bool triggerEvent(int,int){return false;}virtual double getViewDistance(){return 4096;}void setChanged(){++changes;}};
struct BaseEntityTile:Tile {BaseEntityTile(int id,Material*m,bool):Tile(id,m){}void setLightBlock(int){}virtual shared_ptr<TileEntity> newTileEntity(Level*){return{};}virtual bool isSolidRender(bool=false){return false;}virtual int getRenderShape(){return 0;}virtual AABB *getAABB(Level*,int,int,int){return nullptr;}virtual void entityInside(Level*,int,int,int,shared_ptr<Entity>){}virtual int getResourceCount(Random*){return 0;}virtual int getPistonPushReaction(){return 0;}virtual int cloneTileId(Level*,int,int,int){return 0;}virtual void animateTick(Level*,int,int,int,Random*){}};
struct Level {
 static const int maxBuildHeight=256;bool isClientSide=false,flat=false,blocked=false,liquid=false,loaded=true;int seaLevel=63,events=0;int64_t seed=42;Dimension dim,*dimension=&dim;LevelData meta;
 map<tuple<int,int,int>,int> blocks;map<tuple<int,int,int>,shared_ptr<TileEntity>> entities;AABBList cubes;LevelChunk chunk;
 int getTile(int x,int y,int z){auto i=blocks.find({x,y,z});return i==blocks.end()?(flat&&y<=60?121:0):i->second;}
 bool isEmptyTile(int x,int y,int z){return getTile(x,y,z)==0;}
 bool setTileAndData(int,int,int,int,int,int);
 shared_ptr<TileEntity> getTileEntity(int x,int y,int z){return entities[{x,y,z}];}
 void tileEvent(int,int,int,int,int,int){++events;}void addParticle(int,double,double,double,double,double,double){}
 LevelChunk *getChunkAt(int,int){return &chunk;}int getHeightmap(int x,int z){for(int y=255;y>=0;--y)if(getTile(x,y,z))return y+1;return 0;}
 LevelData *getLevelData(){return &meta;}int64_t getSeed(){return seed;}int64_t getGameTime(){return 123456;}
 bool hasChunksAt(int,int,int,int,int,int){return loaded;}
 AABBList *getCubes(shared_ptr<Entity>,AABB *box){cubes.clear();if(blocked){cubes.push_back(box);return &cubes;}for(int x=int(floor(box->x0));x<int(ceil(box->x1));++x)for(int y=int(floor(box->y0));y<int(ceil(box->y1));++y)for(int z=int(floor(box->z0));z<int(ceil(box->z1));++z){int id=getTile(x,y,z);if(id&&id!=209&&id!=119)cubes.push_back(box);}return &cubes;}
 bool containsAnyLiquid(AABB*){return liquid;}
};
struct EndIslandTerrain {static bool isOuterChunk(int x,int z){return int64_t(x)*x+int64_t(z)*z>4096;}};
struct EndOuterFeatures {static void placeSmallIsland(Level *l,Random*,int x,int y,int z){for(int dx=-3;dx<=3;++dx)for(int dz=-3;dz<=3;++dz)l->setTileAndData(x+dx,y,z+dz,121,0,3);}};
'''
TEST=r'''
#include "EndGateway.cpp"
bool Level::setTileAndData(int x,int y,int z,int id,int,int){blocks[{x,y,z}]=id;if(id==209){auto e=make_shared<EndGatewayTileEntity>();e->level=this;e->x=x;e->y=y;e->z=z;entities[{x,y,z}]=e;}else entities.erase({x,y,z});return true;}
int main(){
 Tile bedrock(7,&solidMaterial),stone(121,&solidMaterial),portal(119,&portalMaterial);EndGatewayTile tile(209);
 LevelData root;DerivedLevelData wrapper(&root);wrapper.setEndGatewaysGenerated(21);assert(root.getEndGatewaysGenerated()==20);wrapper.setEndGatewaysGenerated(-1);assert(wrapper.getEndGatewaysGenerated()==0);
 Level w;EndGateways::dragonDefeated(&w);assert(w.meta.getEndGatewaysGenerated()==1);
 auto source=dynamic_pointer_cast<EndGatewayTileEntity>(w.entities.begin()->second);assert(source&&source->age==0);
 int bedrockCount=0;for(auto b:w.blocks)if(b.second==7)++bedrockCount;assert(bedrockCount==12);
 auto player=make_shared<LivingEntity>();player->level=&w;
 assert(!source->transport(player));for(int tick=0;tick<200;++tick)source->tick();
 assert(source->transport(player)&&source->linked&&player->teleports==1&&player->fallDistance==0);
 auto destination=dynamic_pointer_cast<EndGatewayTileEntity>(w.getTileEntity(source->exitX,source->exitY,source->exitZ));
 assert(destination&&destination->linked&&destination->exitX==source->x&&destination->exitZ==source->z);
 assert(!destination->transport(player));for(int tick=0;tick<200;++tick)destination->tick();assert(destination->transport(player)&&player->teleports==2);
 CompoundTag tag;source->save(&tag);EndGatewayTileEntity restored;restored.load(&tag);assert(restored.linked&&restored.exitX==source->exitX&&restored.exitY==source->exitY&&restored.exitZ==source->exitZ);
 auto packet=dynamic_pointer_cast<TileEntityDataPacket>(source->getUpdatePacket());assert(packet&&packet->type==8&&packet->tag->getBoolean(L"HasExit"));
 auto copy=dynamic_pointer_cast<EndGatewayTileEntity>(source->clone());assert(copy&&copy->linked&&copy->level==&w&&copy->exitX==source->exitX);
 tag.putInt(L"ExitX",30000000);restored.load(&tag);assert(!restored.linked);
 w.blocked=true;double x,y,z;assert(!EndGateways::safeExit(&w,source->x,source->y,source->z,player,x,y,z));w.blocked=false;
 w.liquid=true;assert(!EndGateways::safeExit(&w,source->x,source->y,source->z,player,x,y,z));w.liquid=false;
 source->cooldown=0;auto pearl=make_shared<ThrownEnderpearl>();pearl->level=&w;pearl->owner=player;assert(source->transport(pearl)&&pearl->removed&&player->teleports==3);
 source->cooldown=0;player->riding=make_shared<Entity>();assert(!source->transport(player));player->riding.reset();
 w.isClientSide=true;assert(!source->transport(player));assert(!EndGateways::placeFrame(&w,0,75,0));w.isClientSide=false;
 for(int i=1;i<23;++i)EndGateways::dragonDefeated(&w);assert(w.meta.getEndGatewaysGenerated()==20);
 set<pair<int,int>> positions;for(auto e:w.entities){auto [x,y,z]=e.first;if(x*x+z*z<200*200)positions.insert({x,z});}assert(positions.size()==20);
 Level old;old.blocks[{1,63,0}]=119;EndGateways::migrateDefeatedWorld(&old);assert(old.meta.getEndGatewaysGenerated()==1);EndGateways::migrateDefeatedWorld(&old);assert(old.meta.getEndGatewaysGenerated()==1);
 Level active;EndGateways::migrateDefeatedWorld(&active);assert(active.meta.getEndGatewaysGenerated()==0);
 Level client;client.isClientSide=true;client.blocks[{1,63,0}]=119;EndGateways::migrateDefeatedWorld(&client);assert(client.meta.getEndGatewaysGenerated()==0);
}
'''
RENDER_STUBS=r'''
enum {TN_MISC_TUNNEL,TN_MISC_PARTICLEFIELD,TN_MISC_BEACON_BEAM,GL_LIGHTING,GL_BLEND,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_CULL_FACE};
struct ResourceLocation {explicit ResourceLocation(int){}};
inline void glEnable(int){}inline void glDisable(int){}inline void glBlendFunc(int,int){}
struct TileEntityRenderer {virtual ~TileEntityRenderer()=default;virtual void render(shared_ptr<TileEntity>,double,double,double,float,bool,float,bool){}void bindTexture(ResourceLocation*){}};
struct Tesselator {struct Vertex {double x,y,z,u,v;};vector<Vertex> vertices;static Tesselator *getInstance(){static Tesselator t;return &t;}void begin(){}void end(){}void color(float,float,float,float){}void color(int,int,int,int){}void vertexUV(double x,double y,double z,double u,double v){vertices.push_back({x,y,z,u,v});}};
'''
RENDER_TEST=r'''
#include "Minecraft.Client/EndGatewayRenderer.cpp"
int main(){Level w;auto gateway=make_shared<EndGatewayTileEntity>();gateway->level=&w;EndGatewayRenderer renderer;auto t=Tesselator::getInstance();
 for(int age:{0,100,200,10000})for(int cooldown:{0,20,40})for(double cameraPlane:{-1.,0.,1.}){
  gateway->age=age;gateway->cooldown=cooldown;t->vertices.clear();renderer.render(gateway,0,cameraPlane,0,.5f,true,1,true);
  assert(t->vertices.size()>=384);
  for(auto v:t->vertices)assert(isfinite(v.x)&&isfinite(v.y)&&isfinite(v.z)&&isfinite(v.u)&&isfinite(v.v));
  for(size_t i=0;i<384;i+=4){auto a=t->vertices[i],b=t->vertices[i+1],c=t->vertices[i+2];double nx=(b.y-a.y)*(c.z-a.z)-(b.z-a.z)*(c.y-a.y),ny=(b.z-a.z)*(c.x-a.x)-(b.x-a.x)*(c.z-a.z),nz=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);assert(nx*(a.x-.5)+ny*(a.y-cameraPlane-.5)+nz*(a.z-.5)>0);}
 }
}
'''
def method(source,signature):
 start=source.index(signature);opening=source.index('{',start);depth=1;end=opening+1
 while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
 return source[start:end]+'\n'
def main():
 spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 with tempfile.TemporaryDirectory() as temp:
  folder=Path(temp);(folder/'stdafx.h').write_text(STUBS,encoding='utf-8')
  for n in ['TileEntityDataPacket.h','BaseEntityTile.h','TileEntity.h','EndOuterFeatures.h','EndIslandTerrain.h','ChunkSource.h','Random.h','CompoundTag.h','ThrownEnderpearl.h','LevelData.h','net.minecraft.world.entity.player.h','net.minecraft.world.level.h','net.minecraft.world.level.dimension.h','net.minecraft.world.phys.h']:(folder/n).write_text('#include "stdafx.h"\n',encoding='utf-8')
  for n in ['EndGateway.h','EndGateway.cpp']:shutil.copyfile(ROOT/'Minecraft.World'/n,folder/n)
  meta=''
  for name in ['LevelData','DerivedLevelData']:
   source=(ROOT/'Minecraft.World'/(name+'.cpp')).read_text(encoding='utf-8')
   for signature in ['int '+name+'::getEndGatewaysGenerated(', 'void '+name+'::setEndGatewaysGenerated(']:meta+=method(source,signature)
  (folder/'GatewayMeta.h').write_text('#include "stdafx.h"\n'+meta,encoding='utf-8')
  base.compile_and_run(folder,'end_gateway','#include "GatewayMeta.h"\n'+TEST)
  (folder/'Minecraft.World').mkdir();(folder/'Minecraft.Client').mkdir()
  for n in ['EndGateway.h','Level.h','Random.h']:(folder/'Minecraft.World'/n).write_text('#include "../'+('EndGateway.h' if n=='EndGateway.h' else 'stdafx.h')+'"\n',encoding='utf-8')
  for n in ['stdafx.h','TileEntityRenderer.h','Tesselator.h']:(folder/'Minecraft.Client'/n).write_text('#include "../GatewayRenderStubs.h"\n',encoding='utf-8')
  (folder/'GatewayRenderStubs.h').write_text('#pragma once\n#include "stdafx.h"\n'+RENDER_STUBS,encoding='utf-8')
  for n in ['EndGatewayRenderer.h','EndGatewayRenderer.cpp']:shutil.copyfile(ROOT/'Minecraft.Client'/n,folder/'Minecraft.Client'/n)
  # Reuse the same production gateway definitions, with a renderer-specific main.
  definitions=TEST[:TEST.index('int main()')]
  base.compile_and_run(folder,'gateway_geometry','#include "GatewayMeta.h"\n'+definitions+RENDER_TEST)
if __name__=='__main__':main()
