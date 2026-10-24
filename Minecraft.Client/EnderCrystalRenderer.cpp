#include "stdafx.h"
#include "EnderCrystalModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderCrystalRenderer.h"
#include "Tesselator.h"
#include "Lighting.h"

ResourceLocation EnderCrystalRenderer::CRYSTAL_BEAM_LOCATION=ResourceLocation(TN_MOB_ENDERDRAGON_BEAM);
ResourceLocation EnderCrystalRenderer::ENDER_CRYSTAL_LOCATION = ResourceLocation(TN_MOB_ENDERDRAGON_ENDERCRYSTAL);

EnderCrystalRenderer::EnderCrystalRenderer()
{
	currentModel = -1;
	this->shadowRadius = 0.5f;
}

void EnderCrystalRenderer::render(shared_ptr<Entity> _crystal, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type EnderCrystal rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderCrystal> crystal = dynamic_pointer_cast<EnderCrystal>(_crystal);
	if (currentModel != EnderCrystalModel::MODEL_ID)
	{
		model = new EnderCrystalModel(0);
		currentModel = EnderCrystalModel::MODEL_ID;
	}


	float tt = crystal->time + a;
	glPushMatrix();
	glTranslatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	bindTexture(&ENDER_CRYSTAL_LOCATION);
	float hh = sin(tt * 0.2f) / 2 + 0.5f;
	hh = hh * hh + hh;
	model->render(crystal, 0, tt * 3, hh * 0.2f, 0, 0, 1 / 16.0f, true);

	glPopMatrix();
    if(crystal->hasBeamTarget()) {
        float originY=.6f+hh*.4f;
        float dx=float(crystal->getBeamX()+.5-crystal->x),dy=float(crystal->getBeamY()+.5-crystal->y-originY),dz=float(crystal->getBeamZ()+.5-crystal->z);
        float length=std::sqrt(dx*dx+dy*dy+dz*dz),horizontal=std::sqrt(dx*dx+dz*dz);
        if(length<=0)return;
        glPushMatrix();glTranslatef(float(x),float(y)+originY,float(z));
        glRotatef(float(-atan2(dz,dx)*180/PI-90),0,1,0);glRotatef(float(-atan2(horizontal,dy)*180/PI-90),1,0,0);
        glRotatef(tt*.5f,0,0,1);glColor4f(1,1,1,1);
        Lighting::turnOff();glDisable(GL_CULL_FACE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);glShadeModel(GL_SMOOTH);
        bindTexture(&CRYSTAL_BEAM_LOCATION);
        Tesselator *t=Tesselator::getInstance();t->begin(GL_TRIANGLE_STRIP);
        float v0=-tt*.005f,v1=length/32-tt*.005f;
        for(int i=0;i<=8;++i) {
            float angle=float((i%8)*PI*2/8),sx=sin(angle)*.75f,sy=cos(angle)*.75f,u=float(i%8)/8;
            t->vertexUV(sx*.2f,sy*.2f,0,u,v1);t->vertexUV(sx,sy,length,u,v0);
        }
        t->end();glEnable(GL_CULL_FACE);glShadeModel(GL_FLAT);glDisable(GL_BLEND);glPopMatrix();Lighting::turnOn();
    }
}

ResourceLocation *EnderCrystalRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &ENDER_CRYSTAL_LOCATION;
}
