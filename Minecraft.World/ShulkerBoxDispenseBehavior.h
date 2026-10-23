#pragma once
#include "DefaultDispenseItemBehavior.h"
class ShulkerBoxDispenseBehavior:public DefaultDispenseItemBehavior {
protected:
    shared_ptr<ItemInstance> execute(BlockSource*,shared_ptr<ItemInstance>,eOUTCOME&) override;
};
