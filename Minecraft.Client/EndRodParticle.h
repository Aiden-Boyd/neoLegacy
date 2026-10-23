#pragma once
#include "Particle.h"
#include "../Minecraft.World/Random.h"
#include "../Minecraft.World/SharedConstants.h"

class EndRodParticle : public Particle
{
public:
    eINSTANCEOF GetType() override {return eType_ENDRODPARTICLE;}
    EndRodParticle(Level *level,double x,double y,double z,double xa,double ya,double za)
        :Particle(level,x,y,z) {
        xd=xa;yd=ya;zd=za;size*=.75f;noPhysics=false;
        lifetime=60+random->nextInt(12);setColor(1,1,1);setMiscTex(167);
    }
    void tick() override {
        xo=x;yo=y;zo=z;
        if(++age>=lifetime) {remove();return;}
        setMiscTex(160+7-age*8/lifetime);
        if(age>lifetime/2) {
            setAlpha(1.f-(age-lifetime/2)/float(lifetime));
            rCol+=(.95f-rCol)*.2f;gCol+=(.87f-gCol)*.2f;bCol+=(.78f-bCol)*.2f;
        }
        move(xd,yd,zd,true);xd*=.91;yd*=.91;zd*=.91;
    }
    int getLightColor(float) override {return SharedConstants::FULLBRIGHT_LIGHTVALUE;}
    float getBrightness(float) override {return 1;}
};
