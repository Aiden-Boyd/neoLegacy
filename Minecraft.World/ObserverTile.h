#pragma once
#include "Tile.h"
class ObserverTile:public Tile {
    Icon *front,*back,*backOn,*side,*top[4];
    void notifyOutput(Level*,int,int,int,int data);
    void startPulse(Level*,int,int,int);
public:
    explicit ObserverTile(int id);
    void registerIcons(IconRegister*) override;
    Icon *getTexture(int face,int data) override;
    bool isSignalSource() override {return true;}
    int getSignal(LevelSource*,int,int,int,int direction) override;
    int getDirectSignal(LevelSource*l,int x,int y,int z,int direction) override {return getSignal(l,x,y,z,direction);}
    int getSpawnResourcesAuxValue(int) override {return 0;}
    void neighborChanged(Level*,int,int,int,int) override {}
    void onPlace(Level*,int,int,int) override;
    void onRemove(Level*,int,int,int,int,int data) override;
    void tick(Level*,int,int,int,Random*) override;
    void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>) override;
    static void stateChanged(Level*,int x,int y,int z,int flags);
};
