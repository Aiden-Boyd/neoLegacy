"""Compile production box lifecycle/serialization against focused engine fixtures."""
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
SLOTS=r'''
struct Slot {
 shared_ptr<Container>container;int slot=0;
 bool mayPlace(shared_ptr<ItemInstance>);void swap(Slot*);
 int getMaxStackSize(){return container->getMaxStackSize();}void setChanged(){}
 shared_ptr<ItemInstance>getItem(){return container->getItem(slot);}bool hasItem(){return bool(getItem());}void set(shared_ptr<ItemInstance>i){container->setItem(slot,i);}
};
struct ContainerMenu {
 shared_ptr<Container>container;int containerRows=3;vector<Slot*>slots;int moves=0;
 bool moveItemStackTo(shared_ptr<ItemInstance>,int,int,bool){++moves;return true;}
 shared_ptr<ItemInstance>quickMoveStack(shared_ptr<Player>,int);
};
'''
TEST=r'''
int main(){
 for(int color=219;color<=234;++color){
  Level level;ShulkerBoxTile tile(color);level.worldTile=&tile;auto box=make_shared<ShulkerBoxEntity>();box->level=&level;level.te=box;
  box->setCustomName(L"End supplies");auto gems=make_shared<ItemInstance>(264,37,0);auto meta=new CompoundTag();meta->putString(L"secret",L"retained");gems->setTag(meta);box->setItem(26,gems);
  CompoundTag persisted;box->save(&persisted);auto restored=make_shared<ShulkerBoxEntity>();restored->load(&persisted);
  assert(restored->getItem(26)->count==37&&restored->getCustomName()==L"End supplies");assert(restored->getItem(26)->getTag()->getString(L"secret")==L"retained");
  auto clone=dynamic_pointer_cast<ShulkerBoxEntity>(box->clone());clone->getItem(26)->count=1;assert(gems->count==37);
  for(int nested=219;nested<=234;++nested){assert(!box->canPlaceItem(0,make_shared<ItemInstance>(nested,1,0)));box->setItem(0,make_shared<ItemInstance>(nested,1,0));assert(!box->getItem(0));}
  tile.onRemove(&level,0,0,0,color,1);assert(level.drops.size()==1&&box->empty());assert(level.drops[0]->id==color&&level.drops[0]->count==1&&level.drops[0]->getHoverName()==L"End supplies");
  // A repeated stale removal cannot duplicate the item, even if the old TE survives.
  level.te=box;tile.onRemove(&level,0,0,0,color,1);assert(level.drops.size()==1);
  auto placed=make_shared<ShulkerBoxEntity>();placed->level=&level;level.te=placed;tile.setPlacedBy(&level,0,0,0,nullptr,level.drops[0]);
  assert(placed->getItem(26)->count==37&&placed->getCustomName()==L"End supplies");
  placed->getItem(26)->count=12;auto stored=level.drops[0]->getTag()->getCompound(L"BlockEntityTag");auto separate=make_shared<ShulkerBoxEntity>();separate->loadContents(stored);assert(separate->getItem(26)->count==37);
 }
 Level level;ShulkerBoxTile tile(229);level.worldTile=&tile;auto player=make_shared<Player>();
 auto fresh=[&](){auto b=make_shared<ShulkerBoxEntity>();b->level=&level;level.te=b;return b;};
 auto box=fresh();player->abilities.instabuild=true;tile.playerWillDestroy(&level,0,0,0,1,player);tile.onRemove(&level,0,0,0,229,1);assert(level.drops.empty());
 box=fresh();box->setItem(0,make_shared<ItemInstance>(1,64,0));tile.playerWillDestroy(&level,0,0,0,1,player);tile.onRemove(&level,0,0,0,229,1);assert(level.drops.size()==1);
 box=fresh();level.isClientSide=true;tile.onRemove(&level,0,0,0,229,1);assert(level.drops.size()==1);level.isClientSide=false;
 for(int face=0;face<6;++face){box=fresh();level.data=face;assert(tile.getPlacedOnFaceDataValue(&level,0,0,0,face,0,0,0,0)==face);
  level.blocked=true;int opened=player->opened;tile.use(&level,0,0,0,player,1,0,0,0);assert(player->opened==opened);
  level.blocked=false;tile.use(&level,0,0,0,player,1,0,0,0);assert(player->opened==opened+1);
  box->startOpen();for(int n=0;n<10;++n)box->tick();assert(box->openness==1);auto b=tile.getAABB(&level,0,0,0);assert(abs((b->x1-b->x0)*(b->y1-b->y0)*(b->z1-b->z0)-1.5)<.001);
  box->startOpen();box->stopOpen();assert(box->openCount==1);box->stopOpen();for(int n=0;n<10;++n)box->tick();assert(box->openness<.001&&box->openCount==0);
 }
 // Exact lid overlap displacement in every direction, including dropped items
 // (whose native isPushable is false), side/base exclusions and physics guards.
 for(int face=0;face<6;++face){
  box=fresh();level.data=face;box->openCount=1;
  int dx=Facing::STEP_X[face],dy=Facing::STEP_Y[face],dz=Facing::STEP_Z[face];
  auto hit=make_shared<Entity>(),base=make_shared<Entity>(),ghost=make_shared<Entity>();
  hit->bounds={.2,.2,.2,.8,.8,.8};
  if(dx) {hit->bounds.x0=dx>0?1.02:-.08;hit->bounds.x1=dx>0?1.08:-.02;}
  if(dy) {hit->bounds.y0=dy>0?1.02:-.08;hit->bounds.y1=dy>0?1.08:-.02;}
  if(dz) {hit->bounds.z0=dz>0?1.02:-.08;hit->bounds.z1=dz>0?1.08:-.02;}
  ghost->bounds=hit->bounds;ghost->noPhysics=true;
  level.entities={hit,base,ghost};box->tick();
  assert(abs(hit->xd-dx*.04)<.00001&&abs(hit->yd-dy*.04)<.00001&&abs(hit->zd-dz*.04)<.00001);
  assert(base->xd==0&&base->yd==0&&base->zd==0&&ghost->xd==0&&ghost->yd==0&&ghost->zd==0);
  hit->xd=hit->yd=hit->zd=0;hit->removed=true;box->tick();assert(hit->xd==0&&hit->yd==0&&hit->zd==0);
  hit->removed=false;box->openness=.2f;box->openCount=0;box->tick();assert(abs(hit->xd-dx*.04)<.00001&&abs(hit->yd-dy*.04)<.00001&&abs(hit->zd-dz*.04)<.00001);
  hit->xd=hit->yd=hit->zd=0;level.isClientSide=true;box->openCount=1;box->tick();assert(hit->xd==0&&hit->yd==0&&hit->zd==0);level.isClientSide=false;
  level.entities.clear();
 }
 box=fresh();box->setCustomName(L"Named empty box");player->abilities.instabuild=true;
 size_t before=level.drops.size();tile.playerWillDestroy(&level,0,0,0,1,player);tile.onRemove(&level,0,0,0,229,1);assert(level.drops.size()==before+1&&level.drops.back()->getHoverName()==L"Named empty box");
 // Invalid saves cannot insert nested boxes, duplicate slots or overstack tools.
 CompoundTag bad;auto list=new ListTag<CompoundTag>();auto nested=new CompoundTag();nested->putByte(L"Slot",0);ItemInstance(229,1,0).save(nested);list->add(nested);
 for(int count:{99,5}){auto item=new CompoundTag();item->putByte(L"Slot",26);ItemInstance(1,count,0).save(item);list->add(item);}bad.put(L"Items",list);box->loadContents(&bad);assert(!box->getItem(0)&&box->getItem(26)->count==64);
 assert(tile.getResourceCount(nullptr)==0&&tile.getPistonPushReaction()==Material::PUSH_DESTROY);
 auto inventory=make_shared<ChestTileEntity>();auto nestedBox=make_shared<ItemInstance>(229,1,0);inventory->setItem(0,nestedBox);
 auto target=make_shared<ShulkerBoxEntity>();Slot targetSlot;targetSlot.container=target;Slot inventorySlot;inventorySlot.container=inventory;
 assert(!targetSlot.mayPlace(nestedBox)&&inventorySlot.mayPlace(nestedBox));
 targetSlot.swap(&inventorySlot);assert(!target->getItem(0)&&inventory->getItem(0)==nestedBox);
 ContainerMenu menu;menu.container=target;menu.slots.assign(28,&targetSlot);menu.slots[27]=&inventorySlot;
 assert(!menu.quickMoveStack(player,27)&&menu.moves==0&&nestedBox->count==1);assert(!menu.quickMoveStack(player,-1)&&!menu.quickMoveStack(player,28));
 // Client prediction carries the same container type but uses a simple inventory.
 struct ClientBox:ChestTileEntity {int getContainerType()override{return ContainerOpenPacket::SHULKER_BOX;}};
 Slot clientSlot;clientSlot.container=make_shared<ClientBox>();assert(!clientSlot.mayPlace(nestedBox));
 inventory->setItem(0,make_shared<ItemInstance>(264,3,0));assert(menu.quickMoveStack(player,27)&&menu.moves==1);
 ShulkerBoxItem tooltipItem(229-256);auto carried=make_shared<ItemInstance>(229,1,0);auto rootTag=new CompoundTag();auto previewTag=new CompoundTag();auto previewList=new ListTag<CompoundTag>();
 for(int i=0;i<6;++i){auto e=new CompoundTag();e->putByte(L"Slot",i);auto stored=make_shared<ItemInstance>(264,3,0);stored->name=L"Diamonds";stored->save(e);previewList->add(e);}previewTag->put(L"Items",previewList);rootTag->put(L"BlockEntityTag",previewTag);carried->setTag(rootTag);
 vector<HtmlString>lines;tooltipItem.appendHoverText(carried,player,&lines,false);assert(lines.size()==6&&lines[0].text==L"Diamonds x3"&&lines[5].text==L"... and 1 more");
 cout<<"shulker boxes: 16 colors, save/place/break lifecycle, NBT isolation, nested rejection, Creative/client guards and six facing clearances passed\n";
}
'''
def main():
 spec=importlib.util.spec_from_file_location('base',ROOT/'tests/base_stability_regression.py');base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
 with tempfile.TemporaryDirectory() as temp:
  folder=Path(temp);shutil.copyfile(ROOT/'tests/shulker_box_fixture.h',folder/'stdafx.h')
  for name in ['BaseEntityTile.h','ChestTileEntity.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h','net.minecraft.world.entity.player.h','net.minecraft.world.phys.h','ItemInstance.h','ContainerOpenPacket.h','AbstractContainerMenu.h','CompoundTag.h','Facing.h','SoundTypes.h','IconRegister.h','TileItem.h','HtmlString.h']:(folder/name).write_text('#include "stdafx.h"\n')
  for name in ['ShulkerBox.h','ShulkerBox.cpp','ShulkerBoxItem.h','ShulkerBoxItem.cpp']:shutil.copyfile(ROOT/'Minecraft.World'/name,folder/name)
  slot=(ROOT/'Minecraft.World/Slot.cpp').read_text();menu=(ROOT/'Minecraft.World/ContainerMenu.cpp').read_text()
  base.compile_and_run(folder,'shulker_boxes','#include "ShulkerBox.cpp"\n#include "ShulkerBoxItem.cpp"\n'+SLOTS+method(slot,'bool Slot::mayPlace(')+method(slot,'void Slot::swap(')+method(menu,'shared_ptr<ItemInstance> ContainerMenu::quickMoveStack(')+TEST)
  shutil.copyfile(ROOT/'Minecraft.World/SoundTypes.h',folder/'SoundTypes-real.h')
  sounds=(ROOT/'Minecraft.Client/Common/Audio/SoundNames.cpp').read_text()
  start=sounds.index('const WCHAR *ConsoleSoundEngine::wchSoundNames');end=sounds.index('};',start)+2
  array=sounds[start:end].replace('WCHAR','wchar_t').replace('ConsoleSoundEngine::wchSoundNames','names')
  base.compile_and_run(folder,'sound_registry','#include <cassert>\n#include <cwchar>\n#include "SoundTypes-real.h"\n'+array+'\nint main(){for(int i=0;i<eSoundType_MAX;++i)assert(names[i]);assert(!wcscmp(names[eSoundType_SHULKER_AMBIENT],L"entity.shulker.ambient"));assert(!wcscmp(names[eSoundType_SHULKER_BOX_OPEN],L"block.shulker_box.open"));}\n')
if __name__=='__main__':main()
