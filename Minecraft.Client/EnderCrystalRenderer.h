#pragma once
#include "EntityRenderer.h"

class Model;

class EnderCrystalRenderer : public EntityRenderer
{
private:
	int currentModel;
	Model *model;
	static ResourceLocation ENDER_CRYSTAL_LOCATION;
	static ResourceLocation CRYSTAL_BEAM_LOCATION;

public:
	EnderCrystalRenderer();

	virtual void render(shared_ptr<Entity> _crystal, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
};
