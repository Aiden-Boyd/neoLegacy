#pragma once
#include "MobRenderer.h"
#include "EntityRenderer.h"
#include "ModelPart.h"

class ShulkerModel : public Model
{
    ModelPart base,lid,head;
    float partial=0;
public:
    ShulkerModel();
    void prepareMobModel(shared_ptr<LivingEntity>,float,float,float a) override {partial=a;}
    void render(shared_ptr<Entity>,float,float,float,float,float,float,bool) override;
};
class ShulkerRenderer : public MobRenderer
{
    ResourceLocation texture;
protected:
    ResourceLocation *getTextureLocation(shared_ptr<Entity>) override {return &texture;}
    void setupRotations(shared_ptr<LivingEntity>,float,float,float) override;
    int prepareArmor(shared_ptr<LivingEntity>,int,float) override {return -1;}
public:
    ShulkerRenderer();
};
class ShulkerBulletRenderer : public EntityRenderer
{
    Model bulletModel;
    ModelPart bullet;
    ResourceLocation texture;
protected:
    ResourceLocation *getTextureLocation(shared_ptr<Entity>) override {return &texture;}
public:
    ShulkerBulletRenderer();
    void render(shared_ptr<Entity>,double,double,double,float,float) override;
};
