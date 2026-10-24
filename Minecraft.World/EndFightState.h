#pragma once
#include <array>
#include <string>
class CompoundTag;
// One host-owned record, forwarded through DerivedLevelData for the End.
struct EndFightState
{
    enum Phase {IDLE=-1,START=0,PREPARING=1,PILLARS=2,SUMMONING=3,FINISHING=4};
    bool initialized=false,previouslyKilled=false,dragonKilled=false;
    bool dragonChunkKnown=false,pendingAbort=false;
    int dragonChunkX=0,dragonChunkZ=0,missingTicks=0; // missingTicks deliberately restarts on load
    int phase=IDLE,ticks=0,portalY=-1;
    std::wstring dragonId;
    std::array<std::wstring,4> crystals;
    std::array<int,8> pillarHeights{};
    void load(CompoundTag *tag);
    void save(CompoundTag *tag) const;
    void setPhase(int value) {phase=value;ticks=0;}
};
