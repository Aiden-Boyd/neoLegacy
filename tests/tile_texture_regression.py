"""Compile the real tile-entity texture helpers and ResourceLocation dispatch."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from flight_inventory_regression import method
ROOT=Path(__file__).resolve().parents[1]
def stripped(path):return '\n'.join(l for l in (ROOT/path).read_text().splitlines()if not l.startswith('#include')and not l.startswith('#pragma'))
FIXTURE=r'''
#include <cassert>
#include <memory>
#include <string>
using namespace std;
enum _TEXTURE_NAME{TN_CHEST=0,TN_COUNT=1};using TEXTURE_NAME=_TEXTURE_NAME;
template<class T>struct arrayWithLength {T*data=nullptr;unsigned length=0;arrayWithLength(){}arrayWithLength(unsigned n):data(new T),length(n){assert(n==1);}T&operator[](unsigned i){assert(data&&i<length);return data[i];}};
struct intArray{unsigned length=0;int operator[](unsigned){return 0;}};
'''
TAIL=r'''
struct Level{};struct Font{};
struct Textures {int last=-1,indexLoads=0,pathLoads=0,httpIndex=0,httpPath=0;wstring path;
 int loadTexture(int i){assert(i>=0&&i<TN_COUNT);++indexLoads;return 10+i;}int loadTexture(TEXTURE_NAME i,const wstring&p){assert(i==TN_COUNT&&!p.empty());++pathLoads;path=p;return 20;}
 void bind(int i){last=i;}void bindTexture(ResourceLocation*);
 int loadHttpTexture(const wstring&,int i){assert(i>=0&&i<TN_COUNT);++httpIndex;return 30;}int loadHttpTexture(const wstring&,const wstring&p){assert(!p.empty());++httpPath;path=p;return 40;}
 int getHeight(ResourceLocation r){if(r.isPreloaded()){assert(r.getTexture()==TN_CHEST);return 32;}assert(!r.getPath().empty());path=r.getPath();return 64;}
};
struct TileEntityRenderDispatcher {Textures*textures=nullptr;Level*level=nullptr;Font*getFont(){return nullptr;}};
struct TileEntityRenderer {TileEntityRenderDispatcher*tileEntityRenderDispatcher=nullptr;void bindTexture(ResourceLocation*);void bindTexture(const wstring&,ResourceLocation*);int getHeight(ResourceLocation*);Level*getLevel();void init(TileEntityRenderDispatcher*);Font*getFont();};
int main(){Textures textures;TileEntityRenderDispatcher d;d.textures=&textures;TileEntityRenderer r;r.init(&d);ResourceLocation chest(TN_CHEST);r.bindTexture(&chest);assert(textures.last==10&&textures.indexLoads==1);
 const wchar_t*colors[]={L"white",L"orange",L"magenta",L"light_blue",L"yellow",L"lime",L"pink",L"gray",L"silver",L"cyan",L"purple",L"blue",L"brown",L"green",L"red",L"black"};
 for(auto color:colors){ResourceLocation box(wstring(L"mob/shulker_box_")+color+L".png");r.bindTexture(&box);assert(textures.last==20&&textures.path==box.getPath());assert(r.getHeight(&box)==64);r.bindTexture(L"cached",&box);assert(textures.last==40);}
 ResourceLocation banner(wstring(L"mob/end_city_banner.png"));r.bindTexture(&banner);assert(textures.last==20);r.bindTexture(L"cached",&chest);assert(textures.last==30&&textures.httpIndex==1&&textures.httpPath==16);assert(r.getHeight(&chest)==32);r.bindTexture(&chest);assert(textures.last==10);r.bindTexture(nullptr);assert(r.getHeight(nullptr)==32);d.textures=nullptr;r.bindTexture(&banner);assert(r.getHeight(&banner)==32);assert(textures.indexLoads==2&&textures.pathLoads==17);
}
'''
def main():
 # Place implementations ahead of main, preserving native ResourceLocation's empty path-backed array.
 declarations,test=TAIL.split('int main()',1)
 source=FIXTURE+stripped('Minecraft.Client/ResourceLocation.h')+declarations+method('Minecraft.Client/Textures.cpp','void Textures::bindTexture(ResourceLocation')+stripped('Minecraft.Client/TileEntityRenderer.cpp')+'int main()'+test
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'tile_texture_resource_dispatch',source)
 print('All 16 shulker colors, banner, indexed chests, HTTP fallback, height lookup and absent renderer textures: passed')
if __name__=='__main__':main()
