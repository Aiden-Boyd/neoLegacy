#include "stdafx.h"
#include "ShulkerBoxRenderer.h"
#include "../Minecraft.World/ShulkerBox.h"
#include "../Minecraft.World/Tile.h"

ShulkerBoxRenderer::ShulkerBoxRenderer():base(&model,0,28),lid(&model,0,0) {
    model.texWidth=model.texHeight=64;base.setTexSize(64,64);lid.setTexSize(64,64);
    base.addBox(-8,-8,-8,16,8,16);lid.addBox(-8,-16,-8,16,12,16);
    base.setPos(0,24,0);lid.setPos(0,24,0);base.compile(1.f/16.f);lid.compile(1.f/16.f);
    const wchar_t *colors[]={L"white",L"orange",L"magenta",L"light_blue",L"yellow",L"lime",L"pink",L"gray",L"silver",L"cyan",L"purple",L"blue",L"brown",L"green",L"red",L"black"};
    for(int i=0;i<16;++i) textures[i]=ResourceLocation(wstring(L"mob/shulker_box_")+colors[i]+L".png");
}
void ShulkerBoxRenderer::render(shared_ptr<TileEntity> entity,double x,double y,double z,float a,bool,float alpha,bool compiled) {
    auto box=dynamic_pointer_cast<ShulkerBoxEntity>(entity);if(!box)return;
    int color=box->hasLevel()?box->getTile()->id-ShulkerBoxes::FIRST:box->data;
    color=(std::max)(0,(std::min)(15,color));bindTexture(&textures[color]);
    float open=box->oOpenness+(box->openness-box->oOpenness)*a;
    glColor4f(1,1,1,alpha);glPushMatrix();glTranslatef(float(x)+.5f,float(y)+.5f,float(z)+.5f);
    int face=box->hasLevel()?box->getData():1;
    switch(face){case 0:glRotatef(180,1,0,0);break;case 2:glRotatef(-90,1,0,0);break;case 3:glRotatef(90,1,0,0);break;case 4:glRotatef(90,0,0,1);break;case 5:glRotatef(-90,0,0,1);break;}
    glTranslatef(0,1,0);glScalef(1,-1,-1);lid.y=24-open*8;lid.yRot=open*4.712389f;
    base.render(1.f/16.f,compiled);lid.render(1.f/16.f,compiled);glPopMatrix();
}
