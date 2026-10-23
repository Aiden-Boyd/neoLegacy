#pragma once
#include "BaseEntityTile.h"
#include "ChestTileEntity.h"

namespace ShulkerBoxes {
    enum { FIRST=219, LAST=234, PURPLE=229 };
    inline bool isBox(int id) {return id>=FIRST&&id<=LAST;}
}

class ShulkerBoxEntity:public ChestTileEntity {
public:
    bool creativeBreak=false,dropHandled=false;
    ShulkerBoxEntity():ChestTileEntity() {}
    eINSTANCEOF GetType() override {return eTYPE_SHULKERBOXENTITY;}
    static TileEntity *create() {return new ShulkerBoxEntity();}
    int getContainerType() override;
    wstring getName() override;
    bool canPlaceItem(int,shared_ptr<ItemInstance>) override;
    void setItem(unsigned int,shared_ptr<ItemInstance>) override;
    bool empty();
    void loadContents(CompoundTag*);
    void saveContents(CompoundTag*);
    void load(CompoundTag*) override;
    void save(CompoundTag*) override;
    void tick() override;
    void startOpen() override;
    void stopOpen() override;
    bool triggerEvent(int,int) override;
    void checkNeighbors() override {}
    void clearCache() override {TileEntity::clearCache();}
    void setRemoved() override {TileEntity::setRemoved();}
    shared_ptr<TileEntity> clone() override;
};

class ShulkerBoxTile:public BaseEntityTile {
public:
    explicit ShulkerBoxTile(int);
    bool isSolidRender(bool=false) override {return false;}
    bool isCubeShaped() override {return false;}
    int getRenderShape() override {return SHAPE_ENTITYTILE_ANIMATED;}
    int getResourceCount(Random*) override {return 0;}
    int getPistonPushReaction() override;
    shared_ptr<TileEntity> newTileEntity(Level*) override {return std::make_shared<ShulkerBoxEntity>();}
    int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int) override;
    void setPlacedBy(Level*,int,int,int,shared_ptr<LivingEntity>,shared_ptr<ItemInstance>) override;
    bool TestUse() override {return true;}
    bool use(Level*,int,int,int,shared_ptr<Player>,int,float,float,float,bool=false) override;
    void playerWillDestroy(Level*,int,int,int,int,shared_ptr<Player>) override;
    void onRemove(Level*,int,int,int,int,int) override;
    AABB *getAABB(Level*,int,int,int) override;
    void registerIcons(IconRegister*) override;
    bool hasAnalogOutputSignal() override {return true;}
    int getAnalogOutputSignal(Level*,int,int,int,int) override;
};
