#include "stdafx.h"
#include "ShulkerBoxDispenseBehavior.h"
#include "ShulkerBox.h"
#include "BlockSource.h"
#include "DispenserTile.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ItemInstance.h"

shared_ptr<ItemInstance> ShulkerBoxDispenseBehavior::execute(BlockSource *source,shared_ptr<ItemInstance> item,eOUTCOME &outcome) {
    outcome=LEFT_ITEM;
    Level *world=source->getWorld();int face=source->getData()&DispenserTile::FACING_MASK;
    if(world->isClientSide||!item||item->count<1||!ShulkerBoxes::isBox(item->id)||face>5)return item;
    int x=source->getBlockX()+Facing::STEP_X[face],y=source->getBlockY()+Facing::STEP_Y[face],z=source->getBlockZ()+Facing::STEP_Z[face];
    if(y<0||y>=Level::maxBuildHeight)return item;
    if(!world->mayPlace(item->id,x,y,z,false,Facing::DOWN,nullptr,item))return item;
    int orientation=world->isEmptyTile(x,y-1,z)?face:Facing::UP;
    if(!world->setTileAndData(x,y,z,item->id,orientation,Tile::UPDATE_ALL))return item;
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(world->getTileEntity(x,y,z));
    if(!box){world->removeTile(x,y,z);return item;}
    // Restore before consuming, using the same portable-data path as hand placement.
    static_cast<ShulkerBoxTile*>(Tile::tiles[item->id])->setPlacedBy(world,x,y,z,nullptr,item);
    --item->count;box->setChanged();world->updateNeighbourForOutputSignal(x,y,z,item->id);outcome=ACTIVATED_ITEM;
    return item;
}
