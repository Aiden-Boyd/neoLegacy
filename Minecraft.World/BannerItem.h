#pragma once
#include "Item.h"
#include "BannerData.h"
class BannerItem:public Item {
public:
 explicit BannerItem(int id):Item(id){setMaxStackSize(16);setStackedByData(true);}
 int getColor(shared_ptr<ItemInstance>,int) override;
 bool useOn(shared_ptr<ItemInstance>,shared_ptr<Player>,Level*,int,int,int,int,float,float,float,bool=false) override;
};
