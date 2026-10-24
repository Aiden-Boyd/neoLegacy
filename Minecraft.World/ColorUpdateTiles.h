#pragma once
#include "Tile.h"
#include "HeavyTile.h"
class FallingTile;
class ConcreteTile:public Tile {
    Icon *colors[16];
public:
    explicit ConcreteTile(int id);
    void registerIcons(IconRegister*) override;
    Icon *getTexture(int face,int data) override;
    int getSpawnResourcesAuxValue(int data) override;
};
class ConcretePowderTile:public HeavyTile {
    Icon *colors[16];
    bool harden(Level*,int,int,int);
public:
    explicit ConcretePowderTile(int id);
    void registerIcons(IconRegister*) override;
    Icon *getTexture(int face,int data) override;
    int getSpawnResourcesAuxValue(int data) override;
    void onPlace(Level*,int,int,int) override;
    void neighborChanged(Level*,int,int,int,int) override;
    void tick(Level*,int,int,int,Random*) override;
    void onLand(Level*,int,int,int,int) override;
    static bool hardenFalling(FallingTile *entity);
};
class GlazedTerracottaTile:public Tile {
    int color;
    Icon *rotations[4];
public:
    GlazedTerracottaTile(int id,int color);
    void registerIcons(IconRegister*) override;
    Icon *getTexture(int face,int data) override;
    int getSpawnResourcesAuxValue(int) override {return 0;}
    int getPistonPushReaction() override {return 3;} // PUSH_ONLY: movable, not pulled by sticky pistons.
    void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>) override;
    static int textureRotation(int face,int direction);
};
