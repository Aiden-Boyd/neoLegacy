#pragma once
#include "TileItem.h"
class ShulkerBoxItem:public TileItem {
public:
    explicit ShulkerBoxItem(int id):TileItem(id) {setMaxStackSize(1);}
    void appendHoverText(shared_ptr<ItemInstance>,shared_ptr<Player>,vector<HtmlString>*,bool) override;
};
