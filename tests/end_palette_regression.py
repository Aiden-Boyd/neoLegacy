"""Compile End tiles and shared placement/drop methods against a small engine fixture.

This checks metadata and shape contracts, not rendering, network transport, or
real save files. Run the full client and END_UPDATE_TESTS.md for acceptance.
"""
from pathlib import Path
import importlib.util
import shutil
import tempfile

ROOT = Path(__file__).resolve().parents[1]

STUBS = r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
enum { IDS_TILE_PURPUR_SLAB = 100 };
struct Icon { wstring name; };
struct IconRegister {
    map<wstring, Icon> icons;
    Icon* registerIcon(const wstring& name) { return &(icons[name] = Icon{name}); }
};
struct Material { static Material *stone; };
inline Material stoneMaterial;
inline Material *Material::stone = &stoneMaterial;
struct Random {};
struct Entity {};
struct TileEntity {};
struct AABB {};
using AABBList = vector<AABB*>;
struct LevelSource {
    map<tuple<int,int,int>,int> data, blocks;
    int getData(int x,int y,int z) { return data[{x,y,z}]; }
    int getTile(int x,int y,int z) { return blocks[{x,y,z}]; }
};
struct Level : LevelSource {};
struct ItemInstance {
    int id, count, aux;
    ItemInstance(int id,int count,int aux) : id(id),count(count),aux(aux) {}
};
struct Facing {
    enum { DOWN, UP, NORTH, SOUTH, WEST, EAST };
    static int getOpposite(int f) { return f ^ 1; }
};
struct Tile {
    enum { purpurSlab_Id=205, SHAPE_TREE=31 };
    inline static bool solid[256] = {};
    inline static int lightBlock[256] = {};
    inline static Tile* tiles[256] = {};
    int id;
    Material* material;
    Icon* icon = nullptr;
    float y0=0, y1=1;
    Tile(int id,Material* m,bool=true) : id(id),material(m) { tiles[id]=this; }
    virtual ~Tile() = default;
    void setShape(float,float bottom,float,float,float top,float) { y0=bottom; y1=top; }
    virtual int getRenderShape() { return 0; }
    virtual Icon* getTexture(int,int) { return icon; }
    virtual void registerIcons(IconRegister*) {}
    virtual int getResource(int,Random*,int) { return id; }
    virtual int getResourceCount(Random*) { return 1; }
    virtual int getSpawnResourcesAuxValue(int) { return 0; }
    virtual unsigned int getDescriptionId(int=-1) { return 0; }
    virtual void updateShape(LevelSource*,int,int,int,int=-1,shared_ptr<TileEntity> = {}) {}
    virtual void updateDefaultShape() {}
    virtual void addAABBs(Level*,int,int,int,AABB*,AABBList*,shared_ptr<Entity>) {}
    virtual bool isSolidRender(bool=false) { return true; }
    virtual bool isSilkTouchable() { return true; }
    virtual bool isCubeShaped() { return true; }
    virtual bool shouldRenderFace(LevelSource*,int,int,int,int) { return true; }
    virtual int cloneTileData(Level*,int,int,int) { return 0; }
    virtual int cloneTileId(Level*,int,int,int) { return id; }
    virtual int getPlacedOnFaceDataValue(Level*,int,int,int,int,float,float,float,int v) { return v; }
protected:
    virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data) {
        return make_shared<ItemInstance>(id,1,data);
    }
};
'''

TEST = r'''
#include "RotatedPillarTile.cpp"
#include "HalfSlabTile.cpp"
#include "EndBuildingTiles.cpp"
int main() {
    IconRegister icons;
    PurpurPillarTile pillar(202);
    pillar.registerIcons(&icons);
    Level world;
    for (int face=0;face<6;++face) {
        int data=pillar.getPlacedOnFaceDataValue(&world,0,0,0,face,.5f,.5f,.5f,0);
        // Each placement face and its opposite must be the pillar's end grain.
        for (int side=0;side<6;++side) {
            auto texture=pillar.getTexture(side,data);
            assert(texture->name == (side/2 == face/2 ? L"purpur_pillar_top" : L"purpur_pillar"));
        }
        assert(pillar.getSpawnResourcesAuxValue(data)==0); // rotations stack together
    }
    PurpurSlabTile half(205,false), full(204,true);
    assert(!half.isCubeShaped() && full.isCubeShaped());
    assert(half.getResourceCount(nullptr)==1 && full.getResourceCount(nullptr)==2);
    assert(half.getResource(8,nullptr,0)==205 && full.getResource(0,nullptr,0)==205);
    assert(half.getSpawnResourcesAuxValue(8)==0);
    assert(half.cloneTileId(&world,0,0,0)==205);
    world.data[{0,0,0}]=8;
    assert(half.cloneTileData(&world,0,0,0)==0);
    for (int face=0;face<6;++face) for (float clickY : {.25f,.75f}) {
        int data=half.getPlacedOnFaceDataValue(&world,0,0,0,face,.5f,clickY,.5f,0);
        bool upper=face==Facing::DOWN || (face!=Facing::UP && clickY>.5f);
        assert(data==(upper?8:0));
        world.data[{0,0,0}]=data;
        half.updateShape(&world,0,0,0);
        assert(half.y0==(upper?.5f:0.f) && half.y1==(upper?1.f:.5f));
    }
    full.updateDefaultShape();
    assert(full.y0==0.f && full.y1==1.f);
}
'''


def main():
    spec = importlib.util.spec_from_file_location("base_tests", ROOT / "tests/base_stability_regression.py")
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        (folder / "stdafx.h").write_text(STUBS, encoding="utf-8")
        for name in ("Tile.h", "Facing.h", "IconRegister.h", "net.minecraft.h", "net.minecraft.world.level.h", "net.minecraft.world.level.tile.h"):
            (folder / name).write_text('#include "stdafx.h"\n', encoding="utf-8")
        for name in ("EndBuildingTiles", "RotatedPillarTile", "HalfSlabTile"):
            for suffix in (".cpp", ".h"):
                shutil.copyfile(ROOT / "Minecraft.World" / (name + suffix), folder / (name + suffix))
        base.compile_and_run(folder, "end_palette", TEST)


if __name__ == "__main__":
    main()
