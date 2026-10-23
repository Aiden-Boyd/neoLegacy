#pragma once
#include "TileEntityRenderer.h"
class EndCityBannerRenderer : public TileEntityRenderer
{
public:
    void render(shared_ptr<TileEntity>,double,double,double,float,bool,float,bool=true) override;
};
