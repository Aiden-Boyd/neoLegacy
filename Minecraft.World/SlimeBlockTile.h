#pragma once
#include "Tile.h"
#include "Entity.h"
#include "SlimeBlockPhysics.h"
class SlimeBlockTile:public Tile {
public:
 explicit SlimeBlockTile(int id):Tile(id,Material::clay,false){friction=.8f;setLightBlock(0);}
 bool isSolidRender(bool=false) override{return false;}
 int getRenderLayer() override{return 1;}
 void fallOn(Level*,int,int,int,shared_ptr<Entity> e,float) override {if(!e->isSneaking())e->fallDistance=0;}
};
