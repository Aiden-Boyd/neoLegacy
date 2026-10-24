#pragma once
#include "DiggerItem.h"

#define SHOVEL_DIGGABLES 11
class ShovelItem : public DiggerItem
{
private:
	static TileArray *diggables;

public:
	static void staticCtor();
	ShovelItem(int id, const Tier *tier);

	virtual bool useOn(shared_ptr<ItemInstance> instance,shared_ptr<Player> player,Level *level,int x,int y,int z,int face,float clickX,float clickY,float clickZ,bool bTestUseOnOnly=false);
	bool canDestroySpecial(Tile *tile);
};
