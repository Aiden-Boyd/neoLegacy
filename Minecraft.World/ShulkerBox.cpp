#include "stdafx.h"
#include "ShulkerBox.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.phys.h"
#include "ItemInstance.h"
#include "ContainerOpenPacket.h"
#include "AbstractContainerMenu.h"
#include "CompoundTag.h"
#include "Facing.h"
#include "SoundTypes.h"
#include "IconRegister.h"

namespace {
int facing(int data) {return data>=0&&data<6?data:1;}
AABB *boxBounds(int x,int y,int z,int face,double lift) {
    int dx=Facing::STEP_X[face],dy=Facing::STEP_Y[face],dz=Facing::STEP_Z[face];
    return AABB::newTemp(x+(dx<0?-lift:0),y+(dy<0?-lift:0),z+(dz<0?-lift:0),
        x+1+(dx>0?lift:0),y+1+(dy>0?lift:0),z+1+(dz>0?lift:0));
}
}
int ShulkerBoxEntity::getContainerType() {return ContainerOpenPacket::SHULKER_BOX;}
wstring ShulkerBoxEntity::getName() {return hasCustomName()?getCustomName():app.GetString(IDS_TILE_SHULKER_BOX);}
bool ShulkerBoxEntity::canPlaceItem(int slot,shared_ptr<ItemInstance> item) {return slot>=0&&slot<27&&(!item||!ShulkerBoxes::isBox(item->id));}
void ShulkerBoxEntity::setItem(unsigned int slot,shared_ptr<ItemInstance> item) {
    if(slot<27&&canPlaceItem(slot,item)) ChestTileEntity::setItem(slot,item);
}
bool ShulkerBoxEntity::empty() {for(int i=0;i<27;++i) if(getItem(i)) return false;return true;}
void ShulkerBoxEntity::loadContents(CompoundTag *tag) {
    for(int i=0;i<27;++i) ChestTileEntity::setItem(i,nullptr);
    setCustomName(tag->contains(L"CustomName",Tag::TAG_String)?tag->getString(L"CustomName"):L"");
    if(!tag->contains(L"Items",Tag::TAG_List))return;
    auto list=tag->getList(L"Items");
    for(int i=0;i<list->size();++i) {
        if(list->get(i)->getId()!=Tag::TAG_Compound)continue;
        auto entry=static_cast<CompoundTag*>(list->get(i));unsigned int slot=entry->getByte(L"Slot")&255;
        auto item=ItemInstance::fromTag(entry);
        if(slot<27&&item&&item->count>0&&canPlaceItem(slot,item)&&!getItem(slot)) {
            item->count=(std::min)(item->count,item->getMaxStackSize());setItem(slot,item);
        }
    }
}
void ShulkerBoxEntity::saveContents(CompoundTag *tag) {
    auto list=new ListTag<CompoundTag>();
    for(int i=0;i<27;++i) if(auto item=getItem(i)) {
        auto entry=new CompoundTag();entry->putByte(L"Slot",static_cast<byte>(i));item->save(entry);list->add(entry);
    }
    tag->put(L"Items",list);if(hasCustomName())tag->putString(L"CustomName",getCustomName());
}
void ShulkerBoxEntity::load(CompoundTag *tag) {TileEntity::load(tag);loadContents(tag);creativeBreak=dropHandled=false;openCount=0;openness=oOpenness=0;}
void ShulkerBoxEntity::save(CompoundTag *tag) {TileEntity::save(tag);saveContents(tag);}
void ShulkerBoxEntity::startOpen() {
    if(!level||level->isClientSide) return;
    if(openCount<0)openCount=0;++openCount;level->tileEvent(x,y,z,getTile()->id,1,openCount);
    if(openCount==1) level->playSound(x+.5,y+.5,z+.5,eSoundType_SHULKER_BOX_OPEN,.5f,1.f);
}
void ShulkerBoxEntity::stopOpen() {
    if(!level||level->isClientSide||isRemoved()) return;
    openCount=(std::max)(0,openCount-1);level->tileEvent(x,y,z,getTile()->id,1,openCount);
    if(openCount==0) level->playSound(x+.5,y+.5,z+.5,eSoundType_SHULKER_BOX_CLOSE,.5f,1.f);
}
bool ShulkerBoxEntity::triggerEvent(int event,int count) {if(event!=1)return TileEntity::triggerEvent(event,count);openCount=(std::max)(0,count);return true;}
void ShulkerBoxEntity::tick() {
    oOpenness=openness;
    openness=openCount>0?(std::min)(1.f,openness+.1f):(std::max)(0.f,openness-.1f);
    if(level&&!level->isClientSide&&openness>oOpenness) {
        int face=facing(getData());auto entities=*level->getEntities(nullptr,boxBounds(x,y,z,face,openness*.5));
        for(auto entity:entities) if(entity->isPushable())entity->move(Facing::STEP_X[face]*.05,Facing::STEP_Y[face]*.05,Facing::STEP_Z[face]*.05);
    }
}
shared_ptr<TileEntity> ShulkerBoxEntity::clone() {
    auto result=std::make_shared<ShulkerBoxEntity>();TileEntity::clone(result);result->setCustomName(getCustomName());
    for(int i=0;i<27;++i)result->setItem(i,ItemInstance::clone(getItem(i)));
    result->openness=openness;result->oOpenness=oOpenness;result->openCount=openCount;return result;
}
ShulkerBoxTile::ShulkerBoxTile(int id):BaseEntityTile(id,Material::stone,false) {setLightBlock(0);}
int ShulkerBoxTile::getPistonPushReaction() {return Material::PUSH_DESTROY;}
int ShulkerBoxTile::getPlacedOnFaceDataValue(Level*,int,int,int,int face,float,float,float,int) {return facing(face);}
void ShulkerBoxTile::setPlacedBy(Level *level,int x,int y,int z,shared_ptr<LivingEntity>,shared_ptr<ItemInstance> item) {
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z));if(!box)return;
    if(item->hasTag()&&item->getTag()->contains(L"BlockEntityTag",Tag::TAG_Compound))box->loadContents(item->getTag()->getCompound(L"BlockEntityTag"));
    if(item->hasCustomHoverName())box->setCustomName(item->getHoverName());box->setChanged();
}
bool ShulkerBoxTile::use(Level *level,int x,int y,int z,shared_ptr<Player> player,int,float,float,float,bool soundOnly) {
    if(soundOnly||level->isClientSide)return true;
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z));if(!box)return true;
    if(box->openCount==0) {
        int face=facing(level->getData(x,y,z)),dx=Facing::STEP_X[face],dy=Facing::STEP_Y[face],dz=Facing::STEP_Z[face];
        // Only the extra half-block occupied by the lid, excluding the box itself.
        double x0=x,y0=y,z0=z,x1=x+1,y1=y+1,z1=z+1;
        if(dx<0){x0=x-.5;x1=x;}if(dx>0){x0=x+1;x1=x+1.5;}
        if(dy<0){y0=y-.5;y1=y;}if(dy>0){y0=y+1;y1=y+1.5;}
        if(dz<0){z0=z-.5;z1=z;}if(dz>0){z0=z+1;z1=z+1.5;}
        auto space=AABB::newTemp(x0+.0001,y0+.0001,z0+.0001,x1-.0001,y1-.0001,z1-.0001);
        if(!level->getCubes(nullptr,space)->empty())return true;
    }
    player->openContainer(box);return true;
}
void ShulkerBoxTile::playerWillDestroy(Level *level,int x,int y,int z,int,shared_ptr<Player> player) {
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z));if(box)box->creativeBreak=player->abilities.instabuild;
}
void ShulkerBoxTile::onRemove(Level *level,int x,int y,int z,int oldId,int data) {
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z));
    if(!level->isClientSide&&box&&!box->dropHandled) {
        box->dropHandled=true;
        if(!box->creativeBreak||!box->empty()) {
            auto item=std::make_shared<ItemInstance>(id,1,0);auto tag=new CompoundTag();auto contents=new CompoundTag();
            box->saveContents(contents);tag->put(L"BlockEntityTag",contents);item->setTag(tag);
            if(box->hasCustomName())item->setHoverName(box->getCustomName());
            // Empty the live inventory before exposing its serialized replacement.
            for(int slot=0;slot<27;++slot)box->setItem(slot,nullptr);
            popResource(level,x,y,z,item);
        }
        level->updateNeighbourForOutputSignal(x,y,z,id);
    }
    BaseEntityTile::onRemove(level,x,y,z,oldId,data);
}
AABB *ShulkerBoxTile::getAABB(Level *level,int x,int y,int z) {
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z));
    return boxBounds(x,y,z,facing(level->getData(x,y,z)),box?box->openness*.5:0);
}
void ShulkerBoxTile::registerIcons(IconRegister *registry) {icon=registry->registerIcon(L"purpur_block");}
int ShulkerBoxTile::getAnalogOutputSignal(Level *level,int x,int y,int z,int) {
    return AbstractContainerMenu::getRedstoneSignalFromContainer(dynamic_pointer_cast<ShulkerBoxEntity>(level->getTileEntity(x,y,z)));
}
