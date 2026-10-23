#include "stdafx.h"
#include "ShulkerBoxItem.h"
#include "ShulkerBox.h"
#include "ItemInstance.h"
#include "CompoundTag.h"
#include "HtmlString.h"

void ShulkerBoxItem::appendHoverText(shared_ptr<ItemInstance> item,shared_ptr<Player>,vector<HtmlString> *lines,bool) {
    if(!item->hasTag()||!item->getTag()->contains(L"BlockEntityTag",Tag::TAG_Compound))return;
    auto contents=item->getTag()->getCompound(L"BlockEntityTag");
    if(!contents->contains(L"Items",Tag::TAG_List))return;
    auto list=contents->getList(L"Items");int total=0;
    for(int i=0;i<list->size()&&i<27;++i) {
        if(list->get(i)->getId()!=Tag::TAG_Compound)continue;
        auto entry=static_cast<CompoundTag*>(list->get(i));auto stored=ItemInstance::fromTag(entry);
        if(!stored||stored->count<=0||ShulkerBoxes::isBox(stored->id))continue;
        if(total<5)lines->push_back(HtmlString(stored->getHoverName()+L" x"+std::to_wstring(stored->count)));
        ++total;
    }
    if(total>5){wchar_t buffer[128];swprintf_s(buffer,128,app.GetString(IDS_SHULKER_BOX_MORE).c_str(),total-5);lines->push_back(HtmlString(wstring(buffer)));}
}
