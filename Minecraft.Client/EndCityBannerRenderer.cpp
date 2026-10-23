#include "stdafx.h"
#include "EndCityBannerRenderer.h"
#include "../Minecraft.World/TileEntity.h"
#include "../Minecraft.World/Level.h"
#include "ModelPart.h"
#include <cmath>

void EndCityBannerRenderer::render(shared_ptr<TileEntity> tile,double x,double y,double z,float a,bool,float,bool compiled) {
    static ResourceLocation texture(wstring(L"mob/end_city_banner.png"));
    struct BannerModel:Model {
        ModelPart cloth,bar;
        BannerModel():cloth(this,0,0),bar(this,0,42) {
            texWidth=64;texHeight=64;cloth.setTexSize(64,64);bar.setTexSize(64,64);
            cloth.addBox(-10,0,-2,20,40,1);bar.addBox(-10,-2,-2,20,2,2);
        }
    };
    static BannerModel model;
    bindTexture(&texture);glPushMatrix();glTranslatef(float(x)+.5f,float(y)+1,float(z)+.5f);
    int face=tile->getData();float angle=face==2?180.f:face==4?90.f:face==5?-90.f:0.f;
    glRotatef(angle,0,1,0);glTranslatef(0,0,-.4375f);glScalef(.6666667f,-.6666667f,-.6666667f);
    model.cloth.xRot=float(std::cos((tile->level?tile->level->getGameTime()+a:0)*.05+tile->x*7+tile->z*9))*.025f;
    glDisable(GL_CULL_FACE);model.cloth.render(1.f/16.f,compiled);model.bar.render(1.f/16.f,compiled);glEnable(GL_CULL_FACE);glPopMatrix();
}
