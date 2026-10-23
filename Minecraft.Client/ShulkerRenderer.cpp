#include "stdafx.h"
#include "ShulkerRenderer.h"
#include "../Minecraft.World/Shulker.h"
#include <cmath>

ShulkerModel::ShulkerModel():base(this,0,28),lid(this,0,0),head(this,0,52) {
    texWidth=texHeight=64;
    base.setTexSize(64,64);lid.setTexSize(64,64);head.setTexSize(64,64);
    base.addBox(-8,-8,-8,16,8,16);base.setPos(0,24,0);
    lid.addBox(-8,-16,-8,16,12,16);lid.setPos(0,24,0);
    head.addBox(-3,-6,-3,6,6,6);head.setPos(0,16,0);
    base.compile(1.f/16.f);lid.compile(1.f/16.f);head.compile(1.f/16.f);
}
void ShulkerModel::render(shared_ptr<Entity> entity,float,float,float,float yaw,float pitch,float scale,bool compiled) {
    auto shulker=dynamic_pointer_cast<Shulker>(entity);float peek=shulker?shulker->getPeek(partial):0;
    float lift=.5f-.5f*float(std::cos(peek*3.141592653589793));
    lid.y=24-lift*16;lid.yRot=peek>.3f?float(std::sin((peek-.3f)*3.141592653589793))*peek*peek*3.141592653589793f:0;
    head.yRot=yaw*.01745329252f;head.xRot=pitch*.01745329252f;
    base.render(scale,compiled);lid.render(scale,compiled);if(peek>.05f) head.render(scale,compiled);
}
ShulkerRenderer::ShulkerRenderer():MobRenderer(new ShulkerModel(),0),texture(wstring(L"mob/shulker.png")) {}
void ShulkerRenderer::setupRotations(shared_ptr<LivingEntity> mob,float,float,float) {
    auto shulker=dynamic_pointer_cast<Shulker>(mob);if(!shulker) return;
    glTranslatef(0,.5f,0);
    switch(shulker->getAttachFace()) {
    case 1:glRotatef(180,1,0,0);break;case 2:glRotatef(90,1,0,0);break;case 3:glRotatef(-90,1,0,0);break;
    case 4:glRotatef(-90,0,0,1);break;case 5:glRotatef(90,0,0,1);break;
    }
    glTranslatef(0,-.5f,0);
}
ShulkerBulletRenderer::ShulkerBulletRenderer():bullet(&bulletModel,0,0),texture(wstring(L"mob/shulker_spark.png")) {
    bulletModel.texWidth=bulletModel.texHeight=64;bullet.setTexSize(64,64);
    bullet.addBox(-4,-4,-1,8,8,2);bullet.addBox(-1,-4,-4,2,8,8);bullet.addBox(-4,-1,-4,8,2,8);
    bullet.compile(1.f/16.f);
}
void ShulkerBulletRenderer::render(shared_ptr<Entity> entity,double x,double y,double z,float,float a) {
    bindTexture(&texture);glColor4f(1,1,1,1);glPushMatrix();glTranslatef(float(x),float(y)+.15f,float(z));
    glRotatef((entity->tickCount+a)*12,0,1,0);glRotatef((entity->tickCount+a)*8,1,0,0);
    glDisable(GL_LIGHTING);bullet.render(1.f/16.f,true);glEnable(GL_LIGHTING);glPopMatrix();
}
