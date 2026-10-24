"""Exercise Elytra through actual client/server use methods and native menu transfers."""
from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from flight_inventory_regression import method
ROOT=Path(__file__).resolve().parents[1]
FIXTURE=r'''
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
using namespace std;
struct Level{bool isClientSide=false;};struct Player;struct ItemInstance;
struct LivingEntity {enum{SLOT_BOOTS=1,SLOT_LEGGINGS=2,SLOT_CHEST=3,SLOT_HELM=4};};
struct Item {enum{elytra_Id=443,skull_Id=397};virtual~Item(){};};
struct ArmorItem:Item {enum{SLOT_HEAD=0,SLOT_TORSO=1,SLOT_LEGS=2,SLOT_FEET=3};int slot;ArmorItem(int s):slot(s){}};
struct Tile{enum{pumpkin_Id=86};};
struct ElytraItem:Item {bool TestUse(shared_ptr<ItemInstance>,Level*,shared_ptr<Player>);shared_ptr<ItemInstance>use(shared_ptr<ItemInstance>,Level*,shared_ptr<Player>);};
struct ItemInstance:enable_shared_from_this<ItemInstance> {
 int id,count,aux=0;wstring name;vector<int>enchantments;Item*item;
 ItemInstance(int id=443,int n=1,Item*p=nullptr):id(id),count(n),item(p){}
 shared_ptr<ItemInstance>copy(){auto p=make_shared<ItemInstance>(id,count,item);p->aux=aux;p->name=name;p->enchantments=enchantments;return p;}
 Item*getItem(){return item;}int getUseDuration(){return 0;}int getAuxValue(){return aux;}void setAuxValue(int n){aux=n;}bool isDamageableItem(){return true;}bool isStackable(){return count>1&&id!=443;}bool isStackedByData(){return true;}int getMaxStackSize(){return id==443?1:64;}
 static bool tagMatches(shared_ptr<ItemInstance>a,shared_ptr<ItemInstance>b){return a->name==b->name&&a->enchantments==b->enchantments;}
 shared_ptr<ItemInstance>use(Level*l,shared_ptr<Player>p){return dynamic_cast<ElytraItem*>(item)->use(shared_from_this(),l,p);}
 bool TestUse(shared_ptr<ItemInstance>i,Level*l,shared_ptr<Player>p){return dynamic_cast<ElytraItem*>(item)->TestUse(i,l,p);}
};
struct Items {unsigned length=36;vector<shared_ptr<ItemInstance>>data=vector<shared_ptr<ItemInstance>>(36);shared_ptr<ItemInstance>&operator[](unsigned i){return data.at(i);}};
struct Inventory {Items items;vector<shared_ptr<ItemInstance>>armor=vector<shared_ptr<ItemInstance>>(4);int selected=0;bool changed=false;
 void setItem(unsigned i,shared_ptr<ItemInstance>s){if(i>=items.length)armor.at(i-items.length)=s;else items[i]=s;}shared_ptr<ItemInstance>getItem(unsigned i){return i>=items.length?armor.at(i-items.length):items[i];}
 shared_ptr<ItemInstance>getSelected(){return items[selected];}void setChanged(){changed=true;}
};
struct Player:enable_shared_from_this<Player> {shared_ptr<Inventory>inventory=make_shared<Inventory>();bool allowed=true;int sounds=0;void*inventoryMenu=nullptr;virtual~Player(){};
 shared_ptr<ItemInstance>getCarried(int n){return n==0?inventory->getSelected():inventory->armor.at(n-1);}void playSound(int,float,float){++sounds;}bool isAllowedToUse(shared_ptr<ItemInstance>){return allowed;}bool isUsingItem(){return false;}
};
struct MiniStub{bool enabled(){return false;}};struct ServerStub{MiniStub mini;MiniStub*getTumble(){return &mini;}};inline ServerStub disabledServer;
struct ServerPlayer:Player {ServerStub*server=&disabledServer;int refreshes=0;void refreshContainer(void*){++refreshes;}};
struct UseItemPacket {shared_ptr<ItemInstance>hand;UseItemPacket(int,int,int,int,shared_ptr<ItemInstance>h,int,int,int):hand(h){}};
struct Connection {int packets=0;shared_ptr<ItemInstance>predictedHand;void send(shared_ptr<UseItemPacket>p){++packets;predictedHand=p->hand;}};
struct MultiPlayerGameMode {Connection wire,*connection=&wire;void ensureHasSentCarriedItem(){}bool useItem(shared_ptr<Player>,Level*,shared_ptr<ItemInstance>,bool);};
struct ServerPlayerGameMode {shared_ptr<ServerPlayer>player;bool creative=false;bool isCreative(){return creative;}bool useItem(shared_ptr<Player>,Level*,shared_ptr<ItemInstance>,bool);};
struct Slot {Inventory*inv;unsigned number;int changed=0;Slot(Inventory*i,unsigned n):inv(i),number(n){}virtual~Slot(){};virtual bool mayPlace(shared_ptr<ItemInstance>){return true;}virtual int getMaxStackSize()const{return 64;}
 shared_ptr<ItemInstance>getItem(){return inv->getItem(number);}bool hasItem(){return bool(getItem());}void set(shared_ptr<ItemInstance>s){inv->setItem(number,s);}void setChanged(){++changed;}void onTake(shared_ptr<Player>,shared_ptr<ItemInstance>){}void onQuickCraft(shared_ptr<ItemInstance>,shared_ptr<ItemInstance>){}
};
struct ArmorSlot:Slot {int slotNum;ArmorSlot(int n,Inventory*i,unsigned index):Slot(i,index),slotNum(n){}int getMaxStackSize()const{return 1;}bool mayPlace(shared_ptr<ItemInstance>);};
struct ArmorRecipes {enum _eArmorType{eArmorType_None,eArmorType_Helmet,eArmorType_Chestplate,eArmorType_Leggings,eArmorType_Boots};static _eArmorType GetArmorType(int id){return id==443?eArmorType_Chestplate:eArmorType_None;}};
struct AbstractContainerMenu {vector<Slot*>slots;bool moveItemStackTo(shared_ptr<ItemInstance>,int,int,bool);};
struct InventoryMenu:AbstractContainerMenu {enum{RESULT_SLOT=0,ARMOR_SLOT_START=5,INV_SLOT_START=9,INV_SLOT_END=36,USE_ROW_SLOT_START=36,USE_ROW_SLOT_END=45};shared_ptr<ItemInstance>quickMoveStack(shared_ptr<Player>,int);
 InventoryMenu(Inventory*i){for(int n=0;n<5;++n)slots.push_back(new Slot(i,n));for(int n=0;n<4;++n)slots.push_back(new ArmorSlot(n,i,39-n));for(int n=9;n<36;++n)slots.push_back(new Slot(i,n));for(int n=0;n<9;++n)slots.push_back(new Slot(i,n));}~InventoryMenu(){for(auto s:slots)delete s;}
};
'''
TEST=r'''
shared_ptr<ItemInstance> wings(ElytraItem&e){auto i=make_shared<ItemInstance>(443,1,&e);i->aux=127;i->name=L"Named wings";i->enchantments={70,34};return i;}
int main(){
 ElytraItem e;Level clientWorld,host;clientWorld.isClientSide=true;
 {auto p=make_shared<Player>();auto i=wings(e);p->inventory->items[0]=i;MultiPlayerGameMode gm;assert(gm.useItem(p,&clientWorld,i,true));assert(p->sounds==0&&p->inventory->armor[2]==nullptr&&i->count==1&&gm.wire.packets==0);assert(gm.useItem(p,&clientWorld,i,false));assert(!p->inventory->getSelected()&&p->inventory->armor[2]&&p->sounds==1&&gm.wire.packets==1&&!gm.wire.predictedHand);assert(p->inventory->armor[2]!=i&&p->inventory->armor[2]->count==1&&p->inventory->armor[2]->aux==127&&p->inventory->armor[2]->name==L"Named wings"&&p->inventory->armor[2]->enchantments==vector<int>({70,34}));}
 for(bool creative:{false,true}){auto p=make_shared<ServerPlayer>();auto i=wings(e);p->inventory->items[0]=i;ServerPlayerGameMode gm;gm.player=p;gm.creative=creative;assert(gm.useItem(p,&host,i,false));assert(p->inventory->armor[2]&&p->inventory->armor[2]!=i&&p->inventory->armor[2]->count==1&&p->inventory->changed&&p->sounds==1&&p->refreshes==1);assert(bool(p->inventory->getSelected())==creative);if(creative){assert(p->inventory->getSelected()->count==1);p->inventory->getSelected()->aux=200;assert(p->inventory->armor[2]->aux==127);}for(int n:{0,1,3})assert(!p->inventory->armor[n]);}
 {auto p=make_shared<ServerPlayer>();auto old=wings(e),i=wings(e);p->inventory->armor[2]=old;p->inventory->items[0]=i;assert(!e.TestUse(i,&host,p));ServerPlayerGameMode gm;gm.player=p;assert(!gm.useItem(p,&host,i,false));assert(p->inventory->armor[2]==old&&p->inventory->getSelected()==i&&i->count==1&&p->sounds==0);}
 {auto p=make_shared<Player>();auto i=wings(e);i->count=0;assert(!e.TestUse(i,&host,p));assert(!e.TestUse(nullptr,&host,p));assert(!e.TestUse(wings(e),&host,nullptr));}
 for(int from:{9,36}){auto p=make_shared<Player>();InventoryMenu menu(p->inventory.get());auto i=wings(e);menu.slots[from]->set(i);assert(menu.quickMoveStack(p,from));assert(!menu.slots[from]->hasItem()&&p->inventory->armor[2]&&p->inventory->armor[2]->count==1&&p->inventory->armor[2]->aux==127);assert(menu.quickMoveStack(p,6));assert(!p->inventory->armor[2]);}
 {auto p=make_shared<Player>();InventoryMenu menu(p->inventory.get());auto i=wings(e);for(int n=0;n<4;++n)assert(menu.slots[5+n]->mayPlace(i)==(n==1));menu.slots[6]->set(i);auto other=wings(e);menu.slots[9]->set(other);assert(menu.quickMoveStack(p,9));assert(menu.slots[6]->getItem()==i&&menu.slots[36]->getItem()!=i);}
}
'''
def main():
 source=FIXTURE
 for path,signature in [('Minecraft.World/ElytraItem.cpp','bool ElytraItem::TestUse('),('Minecraft.World/ElytraItem.cpp','shared_ptr<ItemInstance> ElytraItem::use('),('Minecraft.Client/MultiPlayerGameMode.cpp','bool MultiPlayerGameMode::useItem('),('Minecraft.Client/ServerPlayerGameMode.cpp','bool ServerPlayerGameMode::useItem('),('Minecraft.World/ArmorSlot.cpp','bool ArmorSlot::mayPlace('),('Minecraft.World/AbstractContainerMenu.cpp','bool AbstractContainerMenu::moveItemStackTo('),('Minecraft.World/InventoryMenu.cpp','shared_ptr<ItemInstance> InventoryMenu::quickMoveStack(')]:source+='\n'+method(path,signature)
 # Actual ArmorSlot compares item IDs; the fixture's Item needs its native id field.
 source=source.replace('enum{elytra_Id=443,skull_Id=397};virtual','enum{elytra_Id=443,skull_Id=397};int id=443;virtual')
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'elytra_equip_and_menu',source+TEST)
 server=(ROOT/'Minecraft.Client/PlayerConnection.cpp').read_text();assert 'item != nullptr && item->id != Item::elytra_Id && ('in server
 print('Elytra prediction, server authority, Creative retention, metadata, chest-only menu transfers and old swap exclusion: passed')
if __name__=='__main__':main()
