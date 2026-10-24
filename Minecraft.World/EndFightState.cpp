#include "stdafx.h"
#include "EndFightState.h"
#include "com.mojang.nbt.h"
void EndFightState::load(CompoundTag *root)
{
    *this=EndFightState();
    if(!root->contains(L"EndFight",Tag::TAG_Compound))return;
    auto tag=root->getCompound(L"EndFight");
    initialized=tag->getBoolean(L"Initialized");previouslyKilled=tag->getBoolean(L"PreviouslyKilled");dragonKilled=tag->getBoolean(L"DragonKilled");
    phase=tag->getInt(L"Phase");if(phase<IDLE || phase>FINISHING)phase=IDLE;
    ticks=tag->getInt(L"Ticks");if(ticks<0 || ticks>1200)ticks=0;
    portalY=tag->getInt(L"PortalY");if(portalY<1 || portalY>250)portalY=-1;
    dragonId=tag->getString(L"DragonUUID");if(dragonId.size()>64)dragonId.clear();
    for(int i=0;i<4;++i){crystals[i]=tag->getString(L"Crystal"+std::to_wstring(i));if(crystals[i].size()>64)crystals[i].clear();}
    for(int i=0;i<8;++i){int h=tag->getInt(L"Pillar"+std::to_wstring(i));pillarHeights[i]=h>=20&&h<=250?h:0;}
    if(phase!=IDLE)initialized=true; // Interrupted/corrupt rituals are recovered by the host, not silently discarded.
}
void EndFightState::save(CompoundTag *root) const
{
    auto tag=new CompoundTag();
    tag->putBoolean(L"Initialized",initialized);tag->putBoolean(L"PreviouslyKilled",previouslyKilled);tag->putBoolean(L"DragonKilled",dragonKilled);
    tag->putInt(L"Phase",phase);tag->putInt(L"Ticks",ticks);tag->putInt(L"PortalY",portalY);tag->putString(L"DragonUUID",dragonId);
    for(int i=0;i<4;++i)tag->putString(L"Crystal"+std::to_wstring(i),crystals[i]);
    for(int i=0;i<8;++i)tag->putInt(L"Pillar"+std::to_wstring(i),pillarHeights[i]);
    root->putCompound(L"EndFight",tag);
}
