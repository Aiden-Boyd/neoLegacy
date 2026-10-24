#include "stdafx.h"
#include "EndFight.h"
#include "EndFightState.h"
#include "EnderDragon.h"
#include "EnderCrystal.h"
#include "LevelData.h"
#include "SpikeFeature.h"
#include "TheEndPortal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "Dimension.h"
#include "net.minecraft.world.entity.player.h"
#include "SoundTypes.h"
namespace {
// Preserve the native LCE eight-pillar arena, including its two cages.
const int pillarX[8]={40,28,0,-28,-40,-28,0,28};
const int pillarZ[8]={0,28,40,28,0,-28,-40,-28};
const int pillarRadius[8]={2,2,2,3,3,3,4,4};
bool hostEnd(Level *l) {return !l->isClientSide && l->dimension->id==1;}
bool arenaLoaded(Level *l) {return l->hasChunksAt(-52,0,-52,52,0,52);}
shared_ptr<EnderDragon> livingDragon(Level *l)
{
    for(auto &entity:l->getAllEntities()) {
        auto dragon=dynamic_pointer_cast<EnderDragon>(entity);
        if(dragon && !dragon->removed)return dragon; // Includes a dragon in its death animation.
    }
    return nullptr;
}
shared_ptr<EnderCrystal> crystalById(Level *l,const std::wstring &id)
{
    if(id.empty())return nullptr;
    for(auto &entity:l->getAllEntities()) {
        auto crystal=dynamic_pointer_cast<EnderCrystal>(entity);
        if(crystal && !crystal->removed && crystal->getUUID()==id)return crystal;
    }
    return nullptr;
}
bool portalActive(Level *l,int y)
{
    return l->getTile(1,y,0)==Tile::endPortalTile_Id || l->getTile(0,y,1)==Tile::endPortalTile_Id;
}
void initialize(Level *l,EndFightState &s)
{
    if(s.portalY<1 || s.portalY>=Level::genDepth-4)s.portalY=l->seaLevel;
    if(s.initialized)return;
    bool portal=portalActive(l,s.portalY);
    s.previouslyKilled=portal || l->getLevelData()->getEndGatewaysGenerated()>0;
    auto dragon=livingDragon(l);s.dragonKilled=portal && !dragon;
    if(dragon)s.dragonId=dragon->getUUID();
    s.initialized=true;
}
void portal(Level *l,EndFightState &s,bool active)
{
    TheEndPortal::allowAnywhere(true);
    for(int x=-2;x<=2;++x)for(int z=-2;z<=2;++z) {
        if((x==0&&z==0) || x*x+z*z>6)continue;
        l->setTileAndData(x,s.portalY,z,active?Tile::endPortalTile_Id:0,0,Tile::UPDATE_ALL);
    }
    TheEndPortal::allowAnywhere(false);
}
void resetCrystals(Level *l,const EndFightState &s)
{
    for(auto &entity:l->getAllEntities()) {
        auto c=dynamic_pointer_cast<EnderCrystal>(entity);if(!c || c->removed)continue;
        bool ritual=false;for(auto &id:s.crystals)if(!id.empty()&&c->getUUID()==id)ritual=true;
        bool pillar=false;for(int i=0;i<8;++i)if(std::abs(c->x-(pillarX[i]+.5))<5 && std::abs(c->z-(pillarZ[i]+.5))<5)pillar=true;
        if(ritual || pillar){c->clearBeamTarget();c->setInvulnerable(false);}
    }
}
void abortRespawn(Level *l,EndFightState &s)
{
    resetCrystals(l,s);portal(l,s,true);s.setPhase(EndFightState::IDLE);s.crystals.fill(L"");s.dragonKilled=true;
}
bool getRitualCrystals(Level *l,EndFightState &s,std::array<shared_ptr<EnderCrystal>,4> &out)
{
    for(int i=0;i<4;++i) {
        out[i]=crystalById(l,s.crystals[i]);if(!out[i])return false;
        for(int j=0;j<i;++j)if(s.crystals[i]==s.crystals[j])return false;
        if(std::abs(out[i]->x)>5 || std::abs(out[i]->z)>5 || std::abs(out[i]->y-(s.portalY+1))>2)return false;
    }
    return true;
}
void beam(std::array<shared_ptr<EnderCrystal>,4> &crystals,int x,int y,int z)
{for(auto &c:crystals)c->setBeamTarget(x,y,z);}
void ritualSound(Level *l) {l->playSound(0,128,0,eSoundType_MOB_ENDERDRAGON_GROWL,5.0f,0.9f);}
void capturePillars(Level *l,EndFightState &s)
{
    for(int i=0;i<8;++i) {
        int height=0;
        for(int y=Level::genDepth-6;y>=20;--y) {
            int tile=l->getTile(pillarX[i],y,pillarZ[i]);
            if(tile==Tile::unbreakable_Id || tile==Tile::obsidian_Id){height=y+(tile==Tile::obsidian_Id?1:0);break;}
        }
        s.pillarHeights[i]=height>=40?height:(std::min)(Level::genDepth-6,(std::max)(40,l->GetHighestY())+12+i*3);
    }
}
void regeneratePillar(Level *l,EndFightState &s,int index)
{
    int x=pillarX[index],z=pillarZ[index],height=s.pillarHeights[index];
    if(height<40 || height>=Level::genDepth-4)height=(std::min)(Level::genDepth-6,l->seaLevel+12+index*3);
    auto entities=l->getAllEntities();
    for(auto &entity:entities) {
        auto crystal=dynamic_pointer_cast<EnderCrystal>(entity);
        if(crystal && !crystal->removed && std::abs(crystal->x-(x+.5))<=10 && std::abs(crystal->z-(z+.5))<=10)crystal->remove();
    }
    for(int xx=x-10;xx<=x+10;++xx)for(int zz=z-10;zz<=z+10;++zz)
        for(int yy=(std::max)(1,height-10);yy<=(std::min)(Level::genDepth-1,height+10);++yy)
            l->setTileAndData(xx,yy,zz,0,0,Tile::UPDATE_CLIENTS);
    l->explode(nullptr,x+.5,height,z+.5,5.0f,true);
    Random random(l->getSeed() ^ (int64_t(index)*341873128712LL));
    SpikeFeature spike(Tile::endStone_Id);
    spike.placeWithIndex(l,&random,x,height-12-index*3,z,index,pillarRadius[index]);
    for(auto &entity:l->getAllEntities()) {
        auto crystal=dynamic_pointer_cast<EnderCrystal>(entity);
        if(crystal && !crystal->removed && std::abs(crystal->x-(x+.5))<1 && std::abs(crystal->z-(z+.5))<1) {
            crystal->setInvulnerable(true);crystal->setBeamTarget(0,128,0);
        }
    }
}
}
bool EndFight::tryRespawn(Level *l)
{
    if(!hostEnd(l) || !arenaLoaded(l))return false;
    auto &s=l->getLevelData()->getEndFight();initialize(l,s);
    if(s.phase!=EndFightState::IDLE || !s.dragonKilled || !s.previouslyKilled || livingDragon(l) || !portalActive(l,s.portalY))return false;
    const int dx[4]={0,0,-2,2},dz[4]={-2,2,0,0};std::array<std::wstring,4> ids;
    for(int i=0;i<4;++i) {
        auto entities=l->getEntitiesOfClass(typeid(EnderCrystal),AABB::newTemp(dx[i],s.portalY+1,dz[i],dx[i]+1,s.portalY+2,dz[i]+1));
        for(auto &entity:*entities) {
            auto c=dynamic_pointer_cast<EnderCrystal>(entity);if(!c || c->removed || c->getUUID().empty())continue;
            bool used=false;for(int j=0;j<i;++j)if(ids[j]==c->getUUID())used=true;
            if(!used){ids[i]=c->getUUID();break;}
        }
        delete entities;if(ids[i].empty())return false;
    }
    s.crystals=ids;capturePillars(l,s);s.setPhase(EndFightState::START);portal(l,s,false);
    return true;
}
void EndFight::tick(Level *l)
{
    if(!hostEnd(l) || !arenaLoaded(l))return;
    // Keep rituals frozen when every player has left the central arena.
    bool nearby=false;for(auto &p:l->players)if(p && std::abs(p->x)<192 && std::abs(p->z)<192){nearby=true;break;}
    if(!nearby)return;
    auto &s=l->getLevelData()->getEndFight();initialize(l,s);
    if(s.phase==EndFightState::IDLE){if(l->getGameTime()%20==0)tryRespawn(l);return;}
    if(s.phase==EndFightState::FINISHING) {
        auto dragon=livingDragon(l);
        if(!dragon) {
            std::array<shared_ptr<EnderCrystal>,4> crystals;
            if(!getRitualCrystals(l,s,crystals)){abortRespawn(l,s);return;}
            dragon=std::make_shared<EnderDragon>(l);dragon->AddParts();dragon->moveTo(0,128,0,0,0);
            if(!l->addEntity(dragon))return; // Retry without consuming crystals or claiming success.
        }
        s.dragonId=dragon->getUUID();s.dragonKilled=false;
        resetCrystals(l,s);
        std::vector<shared_ptr<EnderCrystal> > consumed;
        for(auto &id:s.crystals)if(auto c=crystalById(l,id))consumed.push_back(c);
        // Remove all four before any blast can kill another ritual crystal.
        for(auto &c:consumed){c->clearBeamTarget();c->remove();}
        for(auto &c:consumed)l->explode(c,c->x,c->y,c->z,6.0f,false);
        s.crystals.fill(L"");s.setPhase(EndFightState::IDLE);return;
    }
    // An externally created/live dragon blocks a second spawn and restores a usable fountain.
    if(livingDragon(l)){abortRespawn(l,s);s.dragonKilled=false;return;}
    std::array<shared_ptr<EnderCrystal>,4> crystals;
    if(!getRitualCrystals(l,s,crystals)){abortRespawn(l,s);return;}
    switch(s.phase) {
    case EndFightState::START:beam(crystals,0,128,0);s.setPhase(EndFightState::PREPARING);return;
    case EndFightState::PREPARING:
        if(s.ticks>=100){s.setPhase(EndFightState::PILLARS);return;}
        if(s.ticks==0 || s.ticks==50 || s.ticks==51 || s.ticks==52 || s.ticks>=95)ritualSound(l);
        break;
    case EndFightState::PILLARS: {
        int index=s.ticks/40;
        if(index>=8){s.setPhase(EndFightState::SUMMONING);return;}
        if(s.ticks%40==0)beam(crystals,pillarX[index],s.pillarHeights[index]+1,pillarZ[index]);
        if(s.ticks%40==39)regeneratePillar(l,s,index);
        break;
    }
    case EndFightState::SUMMONING:
        if(s.ticks==0)beam(crystals,0,128,0);
        if(s.ticks>=100){s.setPhase(EndFightState::FINISHING);return;}
        if(s.ticks<5 || s.ticks>=80)ritualSound(l);
        break;
    default:abortRespawn(l,s);return;
    }
    ++s.ticks;
}
void EndFight::crystalDestroyed(Level *l,EnderCrystal *crystal)
{
    if(!hostEnd(l))return;
    auto &s=l->getLevelData()->getEndFight();
    if(s.phase==EndFightState::IDLE || (s.phase==EndFightState::FINISHING && livingDragon(l)))return;
    for(auto &id:s.crystals)if(id==crystal->getUUID()) {abortRespawn(l,s);return;}
}
int EndFight::deathReward(Level *l,EnderDragon *dragon)
{
    if(!hostEnd(l))return 0;
    auto &s=l->getLevelData()->getEndFight();initialize(l,s);
    if(s.dragonId.empty())s.dragonId=dragon->getUUID();
    if(s.dragonId!=dragon->getUUID() || s.dragonKilled)return 0;
    return s.previouslyKilled?500:12000;
}
bool EndFight::dragonDefeated(Level *l,EnderDragon *dragon)
{
    if(!hostEnd(l))return false;
    auto &s=l->getLevelData()->getEndFight();initialize(l,s);
    if(s.dragonKilled || (!s.dragonId.empty() && s.dragonId!=dragon->getUUID()))return false;
    s.dragonId=dragon->getUUID();s.previouslyKilled=true;s.dragonKilled=true;
    s.setPhase(EndFightState::IDLE);return true;
}
