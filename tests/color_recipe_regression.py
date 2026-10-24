from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from content_recipe_regression import STUB
from flight_inventory_regression import method
ROOT=Path(__file__).resolve().parents[1]
TEST=r'''
int main(){Item red{331},quartz{406},dye{351};Tile observer{218},cobble{4},sand{12},gravel{13};Item::redStone=&red;Item::netherQuartz=&quartz;Item::dye_powder=&dye;Tile::observer=&observer;Tile::cobblestone=&cobble;Tile::sand=&sand;Tile::gravel=&gravel;Recipes r;r.registerContent();assert(r.entries.size()==17);auto obs=dynamic_cast<ShapedRecipy*>(r.entries[0]);assert(obs&&obs->result->id==218&&obs->width==3&&obs->height==3);int recipe[9]={4,4,4,331,406,331,4,4,4};for(int i=0;i<9;++i)assert(obs->ingredients[i]->id==recipe[i]);
 for(int color=0;color<16;++color){auto powder=dynamic_cast<ShapelessRecipy*>(r.entries[1+color]);assert(powder&&powder->result->id==252&&powder->result->count==8&&powder->result->aux==color&&powder->ingredients->size()==9);auto grid=make_shared<CraftingContainer>();for(int i=0;i<4;++i)grid->cells[i]=make_shared<ItemInstance>(12);for(int i=4;i<8;++i)grid->cells[i]=make_shared<ItemInstance>(13);grid->cells[8]=make_shared<ItemInstance>(351,1,15-color);assert(powder->matches(grid,nullptr));grid->cells[8]->aux=color;assert(powder->matches(grid,nullptr)==(color==15-color));grid->cells[8]->aux=15-color;grid->cells[0]->aux=1;assert(!powder->matches(grid,nullptr));grid->cells[0]=nullptr;assert(!powder->matches(grid,nullptr));}
}
'''
def main():
 stub=STUB.replace('struct Item{int id;','struct Item{enum{dye_powder_Id=351};int id;').replace('*bowl;','*bowl,*redStone,*netherQuartz;').replace('struct Tile{int id;','struct Tile{enum{concretePowder_Id=252};int id;').replace('*boneBlock;','*boneBlock,*observer,*cobblestone,*sand,*gravel;')
 s=(ROOT/'Minecraft.World/Recipes.cpp').read_text();start=s.index('    addShapedRecipy(new ItemInstance(Tile::observer');end=s.index('    addShapedRecipy(new ItemInstance(Item::endCrystal',start)
 native=method('Minecraft.World/Recipes.cpp','ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...')+method('Minecraft.World/Recipes.cpp','void Recipes::addShapelessRecipy(ItemInstance *result,...')+method('Minecraft.World/ShapelessRecipy.cpp','bool ShapelessRecipy::matches')
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'observer_and_powder_recipes',stub+'void Recipes::registerContent(){'+s[start:end]+'}\n'+native+TEST)
 f=(ROOT/'Minecraft.World/FurnaceRecipes.cpp').read_text()
 line=next(l for l in f.splitlines()if 'for(int color=0;color<16' in l)
 furnace=r'''#include <cassert>
#include <unordered_map>
using namespace std;
struct Tile{enum{clayHardened_colored_Id=159,glazedTerracottaFirst_Id=235};};
struct ItemInstance{int id,count,aux;ItemInstance(int id,int n=1,int a=0):id(id),count(n),aux(a){}int getAuxValue(){return aux;}};
struct FurnaceRecipes{unordered_map<int,ItemInstance*>recipies;unordered_map<int,float>recipeValue;FurnaceRecipes();void addFurnaceRecipy(ItemInstance*,ItemInstance*,float);ItemInstance*getResult(int,int);};
'''
 furnace+='FurnaceRecipes::FurnaceRecipes(){'+line+'}\n'+method('Minecraft.World/FurnaceRecipes.cpp','void FurnaceRecipes::addFurnaceRecipy(ItemInstance*')+method('Minecraft.World/FurnaceRecipes.cpp','ItemInstance* FurnaceRecipes::getResult(')
 furnace+='int main(){FurnaceRecipes f;assert(f.recipies.size()==16);for(int c=0;c<16;++c){auto r=f.getResult(159,c);assert(r&&r->id==235+c&&r->count==1&&r->aux==0&&f.recipeValue[r->id]==.1f);}}'
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'glazed_native_furnace',furnace)

 ui=(ROOT/'Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp').read_text()
 for c in range(16):assert f'ITEM_AUX(Tile::concrete_Id,{c})'in ui and f'ITEM_AUX(Tile::concretePowder_Id,{c})'in ui and f'ITEM({235+c})'in ui
 assert 'ITEM(Tile::observer_Id)'in ui
 print('Native observer/powder recipes, exact dye/sand metadata, furnace registration and all Creative entries: passed')
if __name__=='__main__':main()
