#pragma once
#include "Item.h"
class EndCrystalItem : public Item
{
public:
    explicit EndCrystalItem(int id);
    virtual bool useOn(shared_ptr<ItemInstance> instance,shared_ptr<Player> player,Level *level,int x,int y,int z,int face,float clickX,float clickY,float clickZ,bool bTestUseOnOnly=false);
    virtual bool isFoil(shared_ptr<ItemInstance> instance);
};
