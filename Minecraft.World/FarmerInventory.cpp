#include "stdafx.h"
#include "FarmerInventory.h"
#include "ItemInstance.h"
#include "com.mojang.nbt.h"
FarmerInventory::FarmerInventory() : slots(8) {}
bool FarmerInventory::accepts(int id) {return id==295 || id==296 || id==297 || id==391 || id==392 || id==434 || id==435;}
int FarmerInventory::add(std::shared_ptr<ItemInstance> item)
{
    if(!item || item->count<=0 || !accepts(item->id)) return 0;
    int remaining=item->count;
    for(auto &slot:slots) if(slot && slot->sameItemWithTags(item)) {
        int amount=(std::min)(remaining,slot->getMaxStackSize()-slot->count);
        if(amount>0){slot->count+=amount;remaining-=amount;}
    }
    for(auto &slot:slots) if(!slot && remaining>0) {
        slot=item->copy();slot->count=(std::min)(remaining,item->getMaxStackSize());remaining-=slot->count;
    }
    return item->count-remaining;
}
int FarmerInventory::count(int id) const {int n=0;for(auto &slot:slots)if(slot && slot->id==id)n+=slot->count;return n;}
int FarmerInventory::plantingItem() const {for(auto &slot:slots)if(slot && (slot->id==295 || slot->id==391 || slot->id==392 || slot->id==435))return slot->id;return 0;}
bool FarmerInventory::consume(int id) {for(auto &slot:slots)if(slot && slot->id==id){if(--slot->count==0)slot.reset();return true;}return false;}
bool FarmerInventory::wantsFood() const {return count(297)<15 && count(391)<60 && count(392)<60 && count(434)<60 && count(296)<45;}
void FarmerInventory::save(CompoundTag *tag)
{
    tag->putByte(L"Willing",willing?1:0);
    auto list=new ListTag<CompoundTag>();
    for(int i=0;i<8;++i) if(slots[i]) {auto entry=new CompoundTag();slots[i]->save(entry);entry->putByte(L"Slot",static_cast<byte>(i));list->add(entry);}
    tag->put(L"Inventory",list);
}
void FarmerInventory::load(CompoundTag *tag)
{
    willing=tag->getByte(L"Willing")!=0;
    slots.assign(8,nullptr);
    if(!tag->contains(L"Inventory",Tag::TAG_List))return;
    auto list=tag->getList(L"Inventory");
    for(int i=0;i<list->size();++i) {
        if(list->get(i)->getId()!=Tag::TAG_Compound)continue;
        auto entry=static_cast<CompoundTag*>(list->get(i));int slot=entry->getByte(L"Slot")&255;
        if(slot>=8 || slots[slot])continue;
        auto item=ItemInstance::fromTag(entry);
        if(!item || item->count<=0 || !accepts(item->id))continue;
        item->count=(std::min)(item->count,item->getMaxStackSize());slots[slot]=item;
    }
}

bool FarmerInventory::enoughFood(int multiplier,bool farmer) const {
    for(auto slot:slots)if(slot && ((slot->id==297 && slot->count>=3*multiplier) ||
        ((slot->id==391||slot->id==392||slot->id==434)&&slot->count>=12*multiplier) ||
        (farmer&&slot->id==296&&slot->count>=9*multiplier)))return true;
    return false;
}
bool FarmerInventory::prepareBreeding() {
    if(willing)return true;
    for(auto &slot:slots)if(slot) {
        int needed=slot->id==297?3:(slot->id==391||slot->id==392||slot->id==434)?12:0;
        if(needed&&slot->count>=needed) {
            slot->count-=needed;if(slot->count==0)slot.reset();willing=true;return true;
        }
    }
    return false;
}
std::shared_ptr<ItemInstance> FarmerInventory::share(bool farmer,bool take) {
    if(!enoughFood(2,farmer))return nullptr;
    for(auto &slot:slots)if(slot) {
        int amount=0;std::shared_ptr<ItemInstance> result;
        if((slot->id==297||slot->id==391||slot->id==392||slot->id==434)&&slot->count>3) {
            amount=slot->count/2;result=slot->copy();result->count=amount;
        } else if(farmer&&slot->id==296&&slot->count>5) {
            amount=(slot->count/2/3)*3;result=std::make_shared<ItemInstance>(297,amount/3,0);
        }
        if(result) {if(take){slot->count-=amount;if(slot->count==0)slot.reset();}return result;}
    }
    return nullptr;
}
