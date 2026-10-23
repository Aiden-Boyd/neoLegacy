#pragma once
#include "TileEntityRenderer.h"
#include "Model.h"
#include "ModelPart.h"
class ShulkerBoxRenderer:public TileEntityRenderer {
    Model model;
    ModelPart base,lid;
    ResourceLocation textures[16];
public:
    ShulkerBoxRenderer();
    void render(shared_ptr<TileEntity>,double,double,double,float,bool,float,bool) override;
};
