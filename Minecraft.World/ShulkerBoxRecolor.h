#pragma once
#include "ShulkerBox.h"
#include "ItemInstance.h"
#include "Item.h"

namespace ShulkerBoxes {
// Console-menu family only. This is not a persisted/network item base type.
inline int craftingFamily(int id,int baseType) {return isBox(id)?-1:baseType;}
inline bool canRecolor(shared_ptr<ItemInstance> box,shared_ptr<ItemInstance> dye) {
    return box&&dye&&box->count>0&&dye->count>0&&isBox(box->id)&&dye->id==Item::dye_powder_Id&&dye->getAuxValue()>=0&&dye->getAuxValue()<16;
}
// Both crafting paths use the actual ingredient instance. Copy all portable
// data (including custom names), never move or share the source's NBT pointer.
inline shared_ptr<ItemInstance> recolor(shared_ptr<ItemInstance> source,int resultId) {
    if(!source||!isBox(source->id)||!isBox(resultId))return nullptr;
    auto result=source->copy();result->id=resultId;result->count=1;result->setAuxValue(0);return result;
}
}
