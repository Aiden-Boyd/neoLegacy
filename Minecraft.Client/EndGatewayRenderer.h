#pragma once
#include "TileEntityRenderer.h"

class EndGatewayRenderer : public TileEntityRenderer
{
public:
    void render(shared_ptr<TileEntity>,double,double,double,float,bool,float=1.f,bool=true) override;
};
