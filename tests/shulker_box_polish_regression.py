"""Exercise actual classic/console recolor code and dispenser execution."""
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
SUPPORT=r'''
#include <cstdarg>
struct RegisteredRecipes {
 enum{ANY_AUX_VALUE=-1};struct Entry{int result,source,sourceAux,dye,dyeAux,group;};vector<Entry>entries;
 void build();
 void addShapelessRecipy(ItemInstance*result,const wchar_t*types,...){
  assert(wstring(types)==L"zzg");va_list args;va_start(args,types);auto source=va_arg(args,ItemInstance*);auto dye=va_arg(args,ItemInstance*);int group=va_arg(args,int);va_end(args);
  assert(result->count==1&&source->count==1&&dye->count==1);entries.push_back({result->id,source->id,source->aux,dye->id,dye->aux,group});delete result;
 }
};
struct CraftingContainer {
 array<shared_ptr<ItemInstance>,9>cells{};
 unsigned getContainerSize(){return 9;}shared_ptr<ItemInstance>getItem(int slot){return cells.at(slot);}
 shared_ptr<ItemInstance>getItem(int x,int y){return cells.at(x+y*3);}
};
struct Recipes {enum{ANY_AUX_VALUE=-1};};
struct ShapelessRecipy {
 ItemInstance*result;vector<ItemInstance*>*ingredients;
 bool matches(shared_ptr<CraftingContainer>,Level*);shared_ptr<ItemInstance>assemble(shared_ptr<CraftingContainer>);
};
struct Inventory {
 array<shared_ptr<ItemInstance>,36>items{};
 shared_ptr<ItemInstance>getResourceItem(int id,int aux=-1){for(auto i:items)if(i&&i->count>0&&i->id==id&&(aux==-1||i->aux==aux))return i;return nullptr;}
 bool removeResource(int id,int aux=-1){for(auto&i:items)if(i&&i->count>0&&i->id==id&&(aux==-1||i->aux==aux)){if(--i->count==0)i=nullptr;return true;}return false;}
 void add(shared_ptr<ItemInstance>i){for(auto&s:items)if(!s){s=i;return;}assert(false);}
};
struct CraftPlayer {Inventory*inventory;};
struct Requirement {int iIngC=2;int iIngValA[2]={1,1};int iIngIDA[2];int iIngAuxValA[2]={-1,0};};
struct BlockSource {
 Level*world;int x=10,y=70,z=20,data=1;
 Level*getWorld(){return world;}int getBlockX(){return x;}int getBlockY(){return y;}int getBlockZ(){return z;}int getData(){return data;}
};
struct DispenseItemBehavior {virtual ~DispenseItemBehavior()=default;};
struct DefaultDispenseItemBehavior:DispenseItemBehavior {
 enum eOUTCOME{ACTIVATED_ITEM,DISPENCED_ITEM,LEFT_ITEM};int lastOutcome=-1;
 shared_ptr<ItemInstance>dispense(BlockSource*s,shared_ptr<ItemInstance>i){eOUTCOME o=DISPENCED_ITEM;auto r=execute(s,i,o);lastOutcome=o;return r;}
 virtual shared_ptr<ItemInstance>execute(BlockSource*,shared_ptr<ItemInstance>i,eOUTCOME&o){o=DISPENCED_ITEM;return i;}
};
struct DispenserTile {
 enum{FACING_MASK=7};
 struct Registry {DispenseItemBehavior*get(Item*){static DefaultDispenseItemBehavior fallback;return &fallback;}};
 inline static Registry REGISTRY;
 DispenseItemBehavior*getDispenseMethod(shared_ptr<ItemInstance>);
};
'''
TEST=r'''
shared_ptr<ItemInstance> filled(int id,wstring name=L"Travel kit"){
 auto source=make_shared<ItemInstance>(id,1,0);source->name=name;auto root=new CompoundTag;auto contents=new CompoundTag;auto list=new ListTag<CompoundTag>;
 auto entry=new CompoundTag;entry->putByte(L"Slot",26);auto gem=make_shared<ItemInstance>(264,37,0);auto custom=new CompoundTag;custom->putString(L"enchantment",L"retained");gem->setTag(custom);gem->save(entry);list->add(entry);contents->put(L"Items",list);root->put(L"BlockEntityTag",contents);root->putString(L"extra",L"also retained");source->setTag(root);return source;
}
void verify(shared_ptr<ItemInstance> result,int target){
 assert(result&&result->id==target&&result->count==1&&result->aux==0&&result->name==L"Travel kit");assert(result->getTag()->getString(L"extra")==L"also retained");
 ShulkerBoxEntity box;box.loadContents(result->getTag()->getCompound(L"BlockEntityTag"));assert(box.getItem(26)->count==37&&box.getItem(26)->getTag()->getString(L"enchantment")==L"retained");
}
int main(){
 RegisteredRecipes registered;registered.build();assert(registered.entries.size()==256);
 const int expectedColors[16]={234,233,232,231,230,229,228,227,226,225,224,223,222,221,220,219};
 for(int i=0;i<256;++i){auto e=registered.entries[i];assert(e.source==219+i/16&&e.sourceAux==-1&&e.dye==351&&e.dyeAux==i%16&&e.result==expectedColors[i%16]&&e.group=='M');}
 assert(MAX_UI_VARIANTS>=registered.entries.size()+1);for(int id=0;id<256;++id)assert(ShulkerBoxes::craftingFamily(id,7)==(id>=219&&id<=234?-1:7));
 for(int sourceId=219;sourceId<=234;++sourceId)for(int dye=0;dye<16;++dye){
  int target=219+15-dye;auto original=filled(sourceId);auto pigment=make_shared<ItemInstance>(351,1,dye);
  assert(ShulkerBoxes::canRecolor(original,pigment));auto colored=ShulkerBoxes::recolor(original,target);verify(colored,target);colored->getTag()->putString(L"extra",L"changed");assert(original->getTag()->getString(L"extra")==L"also retained");
  ItemInstance result(target,1,0),input(sourceId,1,-1),ink(351,1,dye);vector<ItemInstance*>ingredients{&input,&ink};ShapelessRecipy recipe{&result,&ingredients};
  auto grid=make_shared<CraftingContainer>();grid->cells[8]=original;grid->cells[0]=pigment;assert(recipe.matches(grid,nullptr));verify(recipe.assemble(grid),target);assert(!recipe.assemble(nullptr)->hasTag());
  grid->cells[4]=pigment;assert(!recipe.matches(grid,nullptr));grid->cells[4]=nullptr;grid->cells[0]=make_shared<ItemInstance>(351,1,(dye+1)%16);assert(!recipe.matches(grid,nullptr));
  for(auto console:{serverConsume,clientConsume}){
   Inventory inventory;inventory.items[0]=filled(sourceId);auto second=filled(sourceId,L"Second box");inventory.items[1]=second;inventory.items[2]=make_shared<ItemInstance>(351,2,dye);
   CraftPlayer player{&inventory};Requirement req;req.iIngIDA[0]=sourceId;req.iIngIDA[1]=351;req.iIngAuxValA[1]=dye;
   verify(console(&player,&req,make_shared<ItemInstance>(target,1,0)),target);assert(!inventory.items[0]&&inventory.items[1]==second&&inventory.items[2]->count==1);
  }
 }
 assert(!ShulkerBoxes::canRecolor(filled(229),make_shared<ItemInstance>(351,1,16)));assert(!ShulkerBoxes::recolor(filled(229),54));
 DispenserTile registry;
 for(int id=219;id<=234;++id){ShulkerBoxTile tile(id);assert(dynamic_cast<ShulkerBoxDispenseBehavior*>(registry.getDispenseMethod(filled(id))));
  for(int face=0;face<6;++face)for(bool support:{false,true}){
   Level world;world.supported=support;BlockSource source{&world};source.data=face|8;auto item=filled(id);ShulkerBoxDispenseBehavior behavior;
   assert(behavior.dispense(&source,item)==item&&item->count==0&&behavior.lastOutcome==DefaultDispenseItemBehavior::ACTIVATED_ITEM);
   auto box=dynamic_pointer_cast<ShulkerBoxEntity>(world.te);assert(box&&box->getItem(26)->count==37&&box->getCustomName()==L"Travel kit");
   assert(world.data==(support?1:face)&&world.placedX==10+Facing::STEP_X[face]&&world.placedY==70+Facing::STEP_Y[face]&&world.placedZ==20+Facing::STEP_Z[face]);
   box->getItem(26)->count=1;ShulkerBoxEntity detached;detached.loadContents(item->getTag()->getCompound(L"BlockEntityTag"));assert(detached.getItem(26)->count==37);
  }
 }
 assert(!dynamic_cast<ShulkerBoxDispenseBehavior*>(registry.getDispenseMethod(make_shared<ItemInstance>(264,1,0))));
 for(int failure=0;failure<4;++failure){Level world;BlockSource source{&world};ShulkerBoxDispenseBehavior behavior;auto item=filled(229);
  if(failure==0)world.allowPlacement=false;if(failure==1)world.allowSet=false;if(failure==2)world.isClientSide=true;if(failure==3){source.y=255;source.data=1;}
  behavior.dispense(&source,item);assert(item->count==1&&!world.te&&behavior.lastOutcome==DefaultDispenseItemBehavior::LEFT_ITEM);
 }
 cout<<"box polish: 256 recolors across classic and both console paths; 192 placements; NBT/name isolation, extra-ingredient rejection and failure guards passed\n";
}
'''
def main():
 spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 with tempfile.TemporaryDirectory() as temp:
  folder=Path(temp);shutil.copyfile(ROOT/'tests/shulker_box_fixture.h',folder/'stdafx.h')
  for name in ['BaseEntityTile.h','ChestTileEntity.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h','net.minecraft.world.entity.player.h','net.minecraft.world.phys.h','ItemInstance.h','ContainerOpenPacket.h','AbstractContainerMenu.h','CompoundTag.h','Facing.h','SoundTypes.h','IconRegister.h','Item.h','BlockSource.h','DispenserTile.h','DefaultDispenseItemBehavior.h']:(folder/name).write_text('#include "stdafx.h"\n')
  for name in ['ShulkerBox.h','ShulkerBox.cpp','ShulkerBoxRecolor.h','ShulkerBoxDispenseBehavior.h','ShulkerBoxDispenseBehavior.cpp']:shutil.copyfile(ROOT/'Minecraft.World'/name,folder/name)
  source=(ROOT/'Minecraft.World/ShapelessRecipy.cpp').read_text()
  methods=method(source,'bool ShapelessRecipy::matches(')+method(source,'shared_ptr<ItemInstance> ShapelessRecipy::assemble(')
  consume=''
  for label,path,player in [('server','Minecraft.Client/PlayerConnection.cpp','player'),('client','Minecraft.Client/Common/UI/IUIScene_CraftingMenu.cpp','m_pPlayer')]:
   source=(ROOT/path).read_text();start=source.index('// and remove those resources from your inventory')
   body=method(source[start:],'for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)')
   consume+='shared_ptr<ItemInstance> '+label+'Consume(CraftPlayer *'+player+',Requirement *pRecipeIngredientsRequired,shared_ptr<ItemInstance> pTempItemInst){int iRecipe=0;'+body+'return pTempItemInst;}\n'
  source=(ROOT/'Minecraft.World/DispenserTile.cpp').read_text()
  registration=method(source,'DispenseItemBehavior *DispenserTile::getDispenseMethod(')
  source=(ROOT/'Minecraft.World/Recipes.cpp').read_text();recipeLoop=method(source,'for(int source=ShulkerBoxes::FIRST;')
  import re
  header=(ROOT/'Minecraft.Client/Common/UI/IUIScene_CraftingMenu.h').read_text();limit=re.search(r'm_iMaxVSlotC\s*=\s*([^;]+);',header).group(1)
  registry='void RegisteredRecipes::build(){'+recipeLoop+'}\nconstexpr int MAX_UI_VARIANTS='+limit+';\n'
  base.compile_and_run(folder,'shulker_box_polish','#include "ShulkerBox.cpp"\n#include "ShulkerBoxRecolor.h"\n'+SUPPORT+'\n#include "ShulkerBoxDispenseBehavior.cpp"\n'+methods+consume+registration+registry+TEST)
if __name__=='__main__':main()
