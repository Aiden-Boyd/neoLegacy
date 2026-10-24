#include "stdafx.h"
#include "EndCityBannerRenderer.h"
#include "../Minecraft.World/EndCityBanner.h"
#include "../Minecraft.World/Level.h"
#include "ModelPart.h"
#include <cmath>
namespace {
struct BannerModel:Model {
 ModelPart cloth,bar,pole;
 BannerModel():cloth(this,0,0),bar(this,0,42),pole(this,44,0){
  texWidth=texHeight=64;cloth.setTexSize(64,64);bar.setTexSize(64,64);pole.setTexSize(64,64);
  cloth.addBox(-10,0,-2,20,40,1);cloth.setPos(0,-32,0);
  bar.addBox(-10,-32,-1,20,2,2);pole.addBox(-1,-30,-1,2,42,2);
  cloth.compile(1.f/16);bar.compile(1.f/16);pole.compile(1.f/16);
 }
};
BannerModel&model(){static BannerModel m;return m;}
void tint(int color,float alpha){glColor4f(((color>>16)&255)/255.f,((color>>8)&255)/255.f,(color&255)/255.f,alpha);}
}
EndCityBannerRenderer::EndCityBannerRenderer(){(void)model();}
void EndCityBannerRenderer::render(shared_ptr<TileEntity>tile,double x,double y,double z,float a,bool,float alpha,bool compiled){
 auto banner=dynamic_pointer_cast<EndCityBannerEntity>(tile);if(!banner)return;
 auto &m=model();bool standing=tile->level&&tile->level->getTile(tile->x,tile->y,tile->z)==176;
 glPushMatrix();glTranslatef(float(x)+.5f,float(y)+(standing?.5f:.3333333f),float(z)+.5f);
 int face=tile->getData();float angle=standing?-(face&15)*22.5f:face==2?180.f:face==4?90.f:face==5?-90.f:0.f;
 glRotatef(angle,0,1,0);if(!standing)glTranslatef(0,0,-.4375f);glScalef(.6666667f,-.6666667f,-.6666667f);
 m.cloth.xRot=float(std::cos((tile->level?tile->level->getGameTime()+a:0)*.05+tile->x*7+tile->z*9))*.025f;
 glDisable(GL_CULL_FACE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 static ResourceLocation base(wstring(L"mob/banner_base.png"));bindTexture(&base);glColor4f(1,1,1,alpha);
 if(standing)m.pole.render(1.f/16,compiled);m.bar.render(1.f/16,compiled);
 tint(BannerData::color(banner->banner.base),alpha);m.cloth.render(1.f/16,compiled);
 glDepthFunc(GL_LEQUAL);
 int layers=0;
 for(auto &p:banner->banner.patterns){if(++layers>20)break;auto file=BannerData::texture(p.first);if(!file)continue;
  ResourceLocation mask(wstring(L"mob/banner/")+file+L".png");bindTexture(&mask);tint(BannerData::color(p.second),alpha);m.cloth.render(1.f/16,compiled);
 }
 glColor4f(1,1,1,alpha);glDisable(GL_BLEND);glEnable(GL_CULL_FACE);glPopMatrix();
}
