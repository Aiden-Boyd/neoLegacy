#include "stdafx.h"
#include "EndGatewayRenderer.h"
#include "../Minecraft.World/EndGateway.h"
#include "../Minecraft.World/Level.h"
#include "../Minecraft.World/Random.h"
#include "Tesselator.h"
#include <cmath>

void EndGatewayRenderer::render(shared_ptr<TileEntity> tile,double x,double y,double z,float partial,bool,float alpha,bool)
{
    auto gateway=dynamic_pointer_cast<EndGatewayTileEntity>(tile);if(!gateway) return;
    static ResourceLocation sky(TN_MISC_TUNNEL),stars(TN_MISC_PARTICLEFIELD),beam(TN_MISC_BEACON_BEAM);
    Tesselator *t=Tesselator::getInstance();Random random(31100);
    // Explicit face UVs keep the six-sided surface well-defined even when the
    // camera crosses its height. No division by a camera/portal plane distance.
    const float vertices[6][4][3]={
        {{0,0,1},{0,0,0},{1,0,0},{1,0,1}},{{0,1,0},{0,1,1},{1,1,1},{1,1,0}},
        {{1,1,0},{1,0,0},{0,0,0},{0,1,0}},{{0,1,1},{0,0,1},{1,0,1},{1,1,1}},
        {{0,1,0},{0,0,0},{0,0,1},{0,1,1}},{{1,1,1},{1,0,1},{1,0,0},{1,1,0}}
    };
    double time=gateway->level?double(gateway->level->getGameTime())+partial:0;
    glDisable(GL_LIGHTING);glEnable(GL_BLEND);
    for(int layer=0;layer<16;++layer) {
        bindTexture(layer==0?&sky:&stars);
        glBlendFunc(layer==0?GL_SRC_ALPHA:GL_ONE,layer==0?GL_ONE_MINUS_SRC_ALPHA:GL_ONE);
        float brightness=layer==0?.1f:1.f/(17-layer);
        float r=random.nextFloat()*.5f+.1f,g=random.nextFloat()*.5f+.4f,b=random.nextFloat()*.5f+.5f;
        if(layer==0) r=g=b=1;
        float scale=layer==0?.125f:layer==1?.5f:.0625f;
        double angle=(layer*layer*4321+layer*9)*.03490658503988659;
        float cs=float(std::cos(angle)),sn=float(std::sin(angle));
        t->begin();t->color(r*brightness,g*brightness,b*brightness,alpha);
        for(int face=0;face<6;++face) for(int v=0;v<4;++v) {
            float a=vertices[face][v][face<4?0:2]-.5f;
            float c=vertices[face][v][face<2?2:1]-.5f;
            float u=(a*cs-c*sn)*scale+.5f;
            float vv=(a*sn+c*cs)*scale+.5f+float(std::fmod(time*.00003*(layer+1),1.));
            t->vertexUV(x+vertices[face][v][0],y+vertices[face][v][1],z+vertices[face][v][2],u,vv);
        }
        t->end();
    }
    if(gateway->age<200||gateway->cooldown>0) {
        bindTexture(&beam);glBlendFunc(GL_SRC_ALPHA,GL_ONE);glDisable(GL_CULL_FACE);
        float progress=gateway->age<200?(gateway->age+partial)/200.f:(40-gateway->cooldown+partial)/40.f;
        if(progress<0) progress=0;if(progress>1) progress=1;
        double height=std::sin(progress*3.141592653589793)*256.;
        float r=gateway->age<200?1.f:.5f;
        for(int pass=0;pass<2;++pass) {
            double radius=pass==0?.15:.22;
            double bottom=y+.5-height,top=y+.5+height;
            double x0=x+.5-radius,x1=x+.5+radius,z0=z+.5-radius,z1=z+.5+radius;
            double corners[4][2]={{x0,z0},{x1,z0},{x1,z1},{x0,z1}};
            t->begin();t->color(r,0,1,pass==0?.8f:.2f);
            float v0=float(-time*.2),v1=v0+float(height*2);
            for(int side=0;side<4;++side) {
                int next=(side+1)%4;
                t->vertexUV(corners[side][0],bottom,corners[side][1],0,v0);
                t->vertexUV(corners[next][0],bottom,corners[next][1],1,v0);
                t->vertexUV(corners[next][0],top,corners[next][1],1,v1);
                t->vertexUV(corners[side][0],top,corners[side][1],0,v1);
            }
            t->end();
        }
        glEnable(GL_CULL_FACE);
    }
    glDisable(GL_BLEND);glEnable(GL_LIGHTING);
}
