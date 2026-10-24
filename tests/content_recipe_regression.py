"""Compile the real registration calls, vararg parsers and shapeless matcher."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
STUB=r'''
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
struct Item{int id;inline static Item *magmaCream,*netherwart_seeds,*netherbrick,*dye_powder,*beetroot,*beetrootSoup,*bowl;};
struct Tile{int id;inline static Tile *magma,*netherWartBlock,*redNetherBrick,*boneBlock;};
struct ItemInstance{int id,count,aux;ItemInstance(int id,int n=1,int a=0):id(id),count(n),aux(a){}ItemInstance(Item*p,int n=1,int a=0):ItemInstance(p->id,n,a){}ItemInstance(Tile*p,int n=1,int a=0):ItemInstance(p->id,n,a){}int getAuxValue(){return aux;}ItemInstance*copy_not_shared(){return new ItemInstance(*this);}};
struct Recipy{enum _eGroupType{eGroupType_Decoration,eGroupType_Tool,eGroupType_Armour,eGroupType_Structure,eGroupType_Transport,eGroupType_Mechanism,eGroupType_Food};virtual~Recipy()=default;};
struct CraftingContainer{vector<shared_ptr<ItemInstance>>cells=vector<shared_ptr<ItemInstance>>(9);unsigned getContainerSize(){return 9;}shared_ptr<ItemInstance>getItem(int i){return cells[i];}shared_ptr<ItemInstance>getItem(int x,int y){return cells[x+y*3];}};struct Level{};
struct ShapedRecipy:Recipy{int width,height;vector<ItemInstance*>ingredients;ItemInstance*result;ShapedRecipy(int w,int h,ItemInstance**ids,ItemInstance*r,int):width(w),height(h),ingredients(ids,ids+w*h),result(r){}};
struct ShapelessRecipy:Recipy{ItemInstance*result;vector<ItemInstance*>*ingredients;ShapelessRecipy(ItemInstance*r,vector<ItemInstance*>*i,_eGroupType):result(r),ingredients(i){}bool matches(shared_ptr<CraftingContainer>,Level*);};
struct Recipes{static const int ANY_AUX_VALUE=-1;vector<Recipy*>entries;vector<Recipy*>*recipies=&entries;ShapedRecipy*addShapedRecipy(ItemInstance*,...);void addShapelessRecipy(ItemInstance*,...);void registerContent();};
using myMap=unordered_map<wchar_t,ItemInstance*>;
struct App{void DebugPrintf(const char*){assert(false);}}app;
struct ShulkerBoxes{static bool isBox(int id){return id>=219&&id<=234;}};
'''
TEST=r'''
int main(){
 Item cream{378},wart{372},brick{405},dye{351},root{434},soup{436},bowl{281};Item::magmaCream=&cream;Item::netherwart_seeds=&wart;Item::netherbrick=&brick;Item::dye_powder=&dye;Item::beetroot=&root;Item::beetrootSoup=&soup;Item::bowl=&bowl;
 Tile magma{213},wartBlock{214},red{215},bone{216};Tile::magma=&magma;Tile::netherWartBlock=&wartBlock;Tile::redNetherBrick=&red;Tile::boneBlock=&bone;
 Recipes r;r.registerContent();assert(r.entries.size()==7);
 int expected[]={213,214,215,216};for(int n=0;n<4;++n){auto shaped=dynamic_cast<ShapedRecipy*>(r.entries[n]);assert(shaped&&shaped->result->id==expected[n]&&shaped->result->count==1);int a=0,b=0;for(auto i:shaped->ingredients){assert(i);if(n==0)assert(i->id==378);if(n==1)assert(i->id==372);if(n==2){a+=i->id==372;b+=i->id==405;}if(n==3)assert(i->id==351&&i->aux==15);}assert(shaped->width==(n==1||n==3?3:2));assert(shaped->height==shaped->width);if(n==2)assert(a==2&&b==2);}
 auto unpack=dynamic_cast<ShapelessRecipy*>(r.entries[4]);assert(unpack->result->id==351&&unpack->result->count==9&&unpack->result->aux==15);auto grid=make_shared<CraftingContainer>();grid->cells[0]=make_shared<ItemInstance>(216);assert(unpack->matches(grid,nullptr));grid->cells[8]=make_shared<ItemInstance>(216);assert(!unpack->matches(grid,nullptr));
 auto redDye=dynamic_cast<ShapelessRecipy*>(r.entries[5]);assert(redDye->result->id==351&&redDye->result->aux==1&&redDye->result->count==1);
 auto recipe=dynamic_cast<ShapelessRecipy*>(r.entries[6]);assert(recipe->result->id==436&&recipe->ingredients->size()==7);
 // Repeat ingredients must each consume one required entry, in any grid positions.
 grid=make_shared<CraftingContainer>();for(int i=0;i<6;++i)grid->cells[i]=make_shared<ItemInstance>(434);grid->cells[8]=make_shared<ItemInstance>(281);assert(recipe->matches(grid,nullptr));
 grid->cells[7]=make_shared<ItemInstance>(434);assert(!recipe->matches(grid,nullptr));grid->cells[7]=nullptr;grid->cells[5]=nullptr;assert(!recipe->matches(grid,nullptr));grid->cells[5]=make_shared<ItemInstance>(434);grid->cells[8]=make_shared<ItemInstance>(434);assert(!recipe->matches(grid,nullptr));
 grid=make_shared<CraftingContainer>();for(int i:{1,2,3,5,6,7})grid->cells[i]=make_shared<ItemInstance>(434);grid->cells[4]=make_shared<ItemInstance>(281);assert(recipe->matches(grid,nullptr));
}
'''
def verify_recipes():
 source=(W/'Recipes.cpp').read_text();start=source.index('    addShapedRecipy(new ItemInstance(Tile::magma');end=source.index('\t// Ordinary shapeless records',start);registration='void Recipes::registerContent(){\n'+source[start:end]+'\n}\n'
 bodies=method('Recipes.cpp','ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...')+'\n'+method('Recipes.cpp','void Recipes::addShapelessRecipy(ItemInstance *result,...')+'\n'+method('ShapelessRecipy.cpp','bool ShapelessRecipy::matches')
 with tempfile.TemporaryDirectory()as tmp:compile_and_run(Path(tmp),'content_recipes',STUB+registration+bodies+TEST)
if __name__=='__main__':verify_recipes()
