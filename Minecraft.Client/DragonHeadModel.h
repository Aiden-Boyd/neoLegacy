#pragma once
#include "Model.h"
#include "ModelPart.h"
#include <cmath>

// Head geometry and texture coordinates shared with the existing dragon model.
class DragonHeadModel : public Model
{
    ModelPart head,jaw;
public:
    DragonHeadModel():head(this,L"head"),jaw(this,L"jaw") {
        texWidth=texHeight=256;
        setMapTex(L"head.upperhead",112,30);setMapTex(L"head.upperlip",176,44);
        setMapTex(L"jaw.jaw",176,65);setMapTex(L"head.scale",0,0);setMapTex(L"head.nostril",112,0);
        head.setTexSize(256,256);jaw.setTexSize(256,256);
        head.addBox(L"upperlip",-6,-1,-24,12,5,16);head.addBox(L"upperhead",-8,-8,-10,16,16,16);
        head.bMirror=true;head.addBox(L"scale",-5,-12,-4,2,4,6);head.addBox(L"nostril",-5,-3,-22,2,2,4);
        head.bMirror=false;head.addBox(L"scale",3,-12,-4,2,4,6);head.addBox(L"nostril",3,-3,-22,2,2,4);
        jaw.setPos(0,4,-8);jaw.addBox(L"jaw",-6,0,-16,12,4,16);head.addChild(&jaw);
    }
    void render(shared_ptr<Entity>,float,float,float,float yaw,float pitch,float scale,bool compiled) override {
        head.yRot=yaw*.01745329252f;head.xRot=pitch*.01745329252f;jaw.xRot=0;
        glPushMatrix();glTranslatef(0,-.15f,0);glScalef(.75f,.75f,.75f);head.render(scale,compiled);glPopMatrix();
    }
};
