"""Compile native map sampling/LUT and added-cell fallback, without a GPU."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>
using namespace std;
#define byte unsigned char
struct MaterialColor {int id,col;inline static MaterialColor *colors[16];inline static MaterialColor *none,*dirt,*stone,*water;};
struct Material {MaterialColor*color;bool isLiquid(){return color==MaterialColor::water;}};
struct Tile {Material*material;inline static Tile*tiles[256];};
struct Mth {static int floor(double n){return int(std::floor(n));}};
enum{eTYPE_PLAYER};struct Entity {double x=0,z=0;virtual~Entity(){}bool instanceof(int){return true;}};struct Player:Entity{};
struct LevelChunk {int block=251,meta=0;bool mixed=false;bool isEmpty(){return false;}int getHeightmap(int,int){return 64;}int getTile(int,int y,int){return y==64?block:1;}int getData(int x,int y,int z){assert(y>=0);return mixed?((x&1)&&(z&1)?1:2):meta;}};
struct Dimension {int id=0;bool hasCeiling=false;};struct Level {Dimension dim,*dimension=&dim;LevelChunk chunk;LevelChunk*getChunkAt(int,int){return &chunk;}};
struct MapItemSavedData {int dimension=0,scale=0,x=0,z=0;byte colors[128*128]={};struct HoldingPlayer{int step=0;};shared_ptr<HoldingPlayer> hp=make_shared<HoldingPlayer>();shared_ptr<HoldingPlayer>getHoldingPlayer(shared_ptr<Player>){return hp;}void setDirty(int,int,int){}};
struct MapItem {enum{IMAGE_WIDTH=128,IMAGE_HEIGHT=128};void update(Level*,shared_ptr<Entity>,shared_ptr<MapItemSavedData>);};
struct ColourTable {int getColor(int col){return 0x345678+col;}};
struct Minecraft {ColourTable table;static Minecraft*GetInstance(){static Minecraft m;return &m;}ColourTable*getColourTable(){return &table;}};
struct Minimap {enum{w=128};inline static int LUT[256];inline static bool genLUT=true;void reloadColours();};
struct Image {int width;vector<vector<int>> levels;explicit Image(int w):width(w),levels(3){for(int l=0;l<3;++l)levels[l].resize((w>>l)*(2*w>>l));}int getWidth(){return width;}int height=0;int getHeight(){return height?height:width*2;}int*getData(int l){return l<int(levels.size())&&!levels[l].empty()?levels[l].data():nullptr;}};
'''
TEST=r'''
int main(){
 MaterialColor legacy[14];Material materials[14];Tile tiles[256];for(int i=0;i<14;++i){legacy[i]={i,i};MaterialColor::colors[i]=&legacy[i];materials[i]={&legacy[i]};}MaterialColor::none=&legacy[0];MaterialColor::dirt=&legacy[10];MaterialColor::stone=&legacy[11];MaterialColor::water=&legacy[12];for(int i=0;i<256;++i){tiles[i].material=&materials[11];Tile::tiles[i]=&tiles[i];}
 Level world;MapItem item;auto player=make_shared<Player>();
 for(int block:{251,252,235,250,219,234,159,201,208,212,213,214,216,218})for(int color=0;color<16;++color){world.chunk.block=block;world.chunk.meta=color;auto data=make_shared<MapItemSavedData>();for(int t=0;t<16;++t)item.update(&world,player,data);assert(data->colors[64+64*128]/4==MapPalette::block(block,color,11));}
 world.chunk.mixed=true;auto data=make_shared<MapItemSavedData>();data->scale=1;world.chunk.block=251;for(int t=0;t<16;++t)item.update(&world,player,data);assert(data->colors[64+64*128]/4==MapPalette::dye(2));
 Minimap map;map.reloadColours();assert(!Minimap::genLUT);for(int id=1;id<64;++id)for(int shade=0;shade<4;++shade){int c=id<14?Minecraft::GetInstance()->table.getColor(id):MapPalette::rgb(id);int b=MapPalette::brightness(shade);unsigned expected=0xff000000u|(((c&255)*b/255)<<16)|((((c>>8)&255)*b/255)<<8)|(((c>>16)&255)*b/255);assert(static_cast<unsigned>(Minimap::LUT[id*4+shade])==expected);}
 assert(MapPalette::rgb(51)==2430480&&MapPalette::rgb(63)==0&&MapPalette::brightness(3)==135);
 for(int width:{256,512}){
 Image selected(width),defaults(256);for(int l=0;l<3;++l)fill(defaults.levels[l].begin(),defaults.levels[l].end(),0xff123456+l);
 // Existing terrain outside the added-cell list survives byte-for-byte.
 for(int l=0;l<3;++l)selected.levels[l][0]=0xffabcdef;
 int row=AddedTerrainFallback::cells[0][0],col=AddedTerrainFallback::cells[0][1],size=width/16;
 selected.levels[0][row*size*width+col*size]=0x01020304; // even partial alpha marks a supplied cell
 auto original=selected.levels;assert(AddedTerrainFallback::needed(&selected));int n=AddedTerrainFallback::apply(&selected,&defaults);assert(n==int(sizeof(AddedTerrainFallback::cells)/sizeof(AddedTerrainFallback::cells[0]))-1);
 assert(!AddedTerrainFallback::needed(&selected));for(int l=0;l<3;++l){assert(selected.levels[l][0]==original[l][0]);int w=width>>l,s=w/16;for(int y=0;y<s;++y)for(int x=0;x<s;++x)assert(selected.levels[l][(row*s+y)*w+col*s+x]==original[l][(row*s+y)*w+col*s+x]);}
 auto once=selected.levels;AddedTerrainFallback::apply(&selected,&defaults);assert(selected.levels==once);
 }
 for(int width:{256,512}){
 Image selected(width),defaults(256);selected.height=width;defaults.height=256;for(int l=0;l<3;++l)fill(defaults.levels[l].begin(),defaults.levels[l].end(),0xffabcdef+l);
 selected.levels[0][0]=0xff123456;assert(AddedTerrainFallback::needsBanner(&selected));assert(AddedTerrainFallback::applyBanner(&selected,&defaults)==1);assert(selected.levels[0][0]==int(0xff123456));assert(!AddedTerrainFallback::needsBanner(&selected));
 for(int l=0;l<3;++l){int w=width>>l,s=w/16;assert(selected.levels[l][13*s*w+10*s]==int(0xffabcdef)+l);}auto after=selected.levels;assert(AddedTerrainFallback::applyBanner(&selected,&defaults)==0&&after==selected.levels);
 }
 Image low(8);assert(!AddedTerrainFallback::needed(&low));
}
'''
def main():
 s=SUPPORT+'\n'+(ROOT/'Minecraft.World/MapPalette.h').read_text()+'\n'+(ROOT/'Minecraft.Client/AddedTerrainFallback.h').read_text()
 s+='\n'+method('MapItem.cpp','void MapItem::update(')
 src=(ROOT/'Minecraft.Client/Minimap.cpp').read_text();a=src.index('void Minimap::reloadColours()');b=src.index('// 4J added entityId',a);s+='\n'+src[a:b]
 with tempfile.TemporaryDirectory() as d:compile_and_run(Path(d),'map_texture', '#define _WIN64\n'+s+TEST)
if __name__=='__main__':main()
