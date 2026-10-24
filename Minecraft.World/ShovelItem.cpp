#include "stdafx.h"

#include "net.minecraft.world.level.tile.h"
#include "ShovelItem.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "ItemInstance.h"

TileArray *ShovelItem::diggables = nullptr;

void ShovelItem::staticCtor()
{
	ShovelItem::diggables = new TileArray( SHOVEL_DIGGABLES);
	diggables->data[0] = Tile::grass;
	diggables->data[1] = Tile::dirt;
	diggables->data[2] = Tile::sand;
	diggables->data[3] = Tile::gravel;
	diggables->data[4] = Tile::topSnow;
	diggables->data[5] = Tile::snow;
	diggables->data[6] = Tile::clay;
	diggables->data[7] = Tile::farmland;
	diggables->data[8] = Tile::soulsand;
	diggables->data[9] = Tile::mycel;
	diggables->data[10] = Tile::grassPath;
}

ShovelItem::ShovelItem(int id, const Tier *tier) : DiggerItem(id, 1, tier, diggables)
{
}

bool ShovelItem::canDestroySpecial(Tile *tile)
{
	if (tile == Tile::topSnow) return true;
	if (tile == Tile::snow) return true;
	return false;
}

bool ShovelItem::useOn(shared_ptr<ItemInstance> instance,shared_ptr<Player> player,Level *level,int x,int y,int z,int face,float,float,float,bool preview)
{
    if(!instance || instance->count<=0 || !player || !player->mayUseItemAt(x,y,z,face,instance)) return false;
    if(face==0 || y>=255 || level->getTile(x,y,z)!=Tile::grass_Id || level->getTile(x,y+1,z)!=0) return false;
    if(preview) return true;
    if(level->isClientSide) return true;
    if(!level->setTileAndData(x,y,z,Tile::grassPath_Id,0,Tile::UPDATE_ALL)) return false;
    Tile *tile=Tile::grassPath;
    level->playSound(x+.5,y+.5,z+.5,tile->soundType->getStepSound(),1.0f,0.8f);
    instance->hurtAndBreak(1,player);
    return true;
}
