"""Compile actual production methods against a small deterministic engine fixture.

Requires Python 3 and g++/clang++ or an MSVC developer shell. This does not
replace in-game save/reload, rendering, or world-generation acceptance tests.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

STUBS = r'''
#pragma once
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
enum { IDS_TILE_SPONGE, IDS_TILE_WET_SPONGE, IDS_DESC_SPONGE, IDS_DESC_WET_SPONGE };
enum { eParticleType_dripWater };
struct Material {
    static Material *sponge, *water, *air;
};
inline Material solidMaterial, waterMaterial, airMaterial;
inline Material *Material::sponge = &solidMaterial;
inline Material *Material::water = &waterMaterial;
inline Material *Material::air = &airMaterial;
struct Icon {};
struct IconRegister { Icon* registerIcon(const wchar_t*) { return nullptr; } };
struct Random {};
struct Facing {
    enum { DOWN, UP, NORTH, SOUTH, WEST, EAST };
    inline static int STEP_X[6] = {0, 0, 0, 0, -1, 1};
    inline static int STEP_Y[6] = {-1, 1, 0, 0, 0, 0};
    inline static int STEP_Z[6] = {0, 0, -1, 1, 0, 0};
};
struct Tile {
    enum { sponge_Id = 19, UPDATE_CLIENTS = 2 };
    Tile(int, Material*) {}
};
using Position = tuple<int, int, int>;
struct Level {
    map<Position, int> blocks, data;
    int particles = 0, removals = 0;
    bool failWrites = false;
    function<void()> notify;
    int getData(int x, int y, int z) { return data[{x,y,z}]; }
    Material* getMaterial(int x, int y, int z) {
        int id = blocks[{x,y,z}];
        return id == 8 || id == 9 ? Material::water : Material::air;
    }
    bool setTileAndData(int x, int y, int z, int id, int aux, int) {
        if (failWrites) return false;
        blocks[{x,y,z}] = id;
        data[{x,y,z}] = aux;
        return true;
    }
    bool removeTile(int x, int y, int z) {
        blocks[{x,y,z}] = 0;
        ++removals;
        if (notify) notify();
        return true;
    }
    void checkLight(int, int, int) {}
    bool isTopSolidBlocking(int, int, int) { return false; }
    void addParticle(int, double, double, double, double, double, double) { ++particles; }
    void sponge(int x, int aux) { blocks[{x,0,0}] = Tile::sponge_Id; data[{x,0,0}] = aux; }
    void water(int x, int y, int z) { blocks[{x,y,z}] = 9; }
};
'''

SPONGE_TEST = r'''
#include "Sponge.cpp"
struct TestSponge : Sponge {
    TestSponge() : Sponge(19) {}
    using Sponge::onPlace;
    using Sponge::neighborChanged;
    using Sponge::animateTick;
};
int main() {
    TestSponge sponge;
    Level world;
    world.sponge(0, 0); world.water(0, 0, 1);
    sponge.onPlace(&world, 0, 0, 0);
    assert(world.getData(0,0,0) == 1);
    world.sponge(20, 0);
    sponge.onPlace(&world, 20, 0, 0);
    assert(world.getData(20,0,0) == 0); // no water: remain dry

    // A dry placement must not make an existing wet sponge absorb or stop dripping.
    Random random;
    for (int i = 0; i < 80; ++i) sponge.animateTick(&world, 0, 0, 0, &random);
    assert(world.particles > 0);
    world.water(0, 0, 1);
    sponge.neighborChanged(&world, 0, 0, 0, 9);
    assert(world.getMaterial(0,0,1) == Material::water);

    // A wet placement must not prevent a different dry sponge absorbing later.
    world.sponge(40, 1);
    sponge.onPlace(&world, 40, 0, 0);
    world.water(20, 0, 1);
    sponge.neighborChanged(&world, 20, 0, 0, 9);
    assert(world.getData(20,0,0) == 1);
    assert(world.getMaterial(20,0,1) != Material::water);

    // A fresh tile object must read saved wetness, not depend on placement history.
    TestSponge reloaded;
    Level saved;
    saved.sponge(0, 1); saved.water(0, 0, 1);
    reloaded.neighborChanged(&saved, 0, 0, 0, 9);
    assert(saved.getMaterial(0,0,1) == Material::water);
    saved.sponge(20, 0); saved.water(20, 0, 1);
    reloaded.neighborChanged(&saved, 20, 0, 0, 9);
    assert(saved.getData(20,0,0) == 1);

    // Water removal callbacks see wet metadata and cannot reenter absorption.
    Level callbacks;
    callbacks.sponge(0, 0);
    callbacks.water(0,0,1); callbacks.water(1,0,1); callbacks.water(1,0,0);
    callbacks.notify = [&] {
        assert(callbacks.getData(0,0,0) == 1);
        sponge.neighborChanged(&callbacks, 0, 0, 0, 9);
    };
    sponge.onPlace(&callbacks, 0, 0, 0);
    assert(callbacks.removals == 3); // no duplicate visits in a water loop

    // Failed state publication must not remove water.
    Level failure;
    failure.sponge(0,0); failure.water(0,0,1); failure.failWrites = true;
    sponge.onPlace(&failure, 0, 0, 0);
    assert(failure.removals == 0);

    // A wet sponge in another world must not influence this world's dry sponge.
    Level other;
    other.sponge(0,1); sponge.onPlace(&other,0,0,0);
    Level fresh;
    fresh.sponge(0,0); fresh.water(0,0,1);
    sponge.neighborChanged(&fresh,0,0,0,9);
    assert(fresh.getData(0,0,0) == 1);
}
'''

MONUMENT_FIXTURE = r'''
#include <cassert>
#include <vector>
using byteArray = std::vector<unsigned char>;
struct Feature {
    int calls = 0, x = 0, z = 0;
    void apply(void*, void*, int cx, int cz, byteArray blocks) {
        assert(blocks.empty()); // reconstruct metadata, never place blocks here
        ++calls; x = cx; z = cz;
    }
};
struct RandomLevelSource {
    bool generateStructures = true;
    void* level = nullptr;
    Feature *mineShaftFeature, *villageFeature, *strongholdFeature, *scatteredFeature, *oceanMonument;
    void recreateLogicStructuresForChunk(int, int);
};
'''

MONUMENT_TEST = r'''
int main() {
    Feature features[5];
    RandomLevelSource source;
    source.mineShaftFeature = &features[0]; source.villageFeature = &features[1];
    source.strongholdFeature = &features[2]; source.scatteredFeature = &features[3];
    source.oceanMonument = &features[4];
    source.recreateLogicStructuresForChunk(-35, 72);
    for (const auto& f : features) assert(f.calls == 1 && f.x == -35 && f.z == 72);
    source.generateStructures = false;
    source.recreateLogicStructuresForChunk(0, 0);
    for (const auto& f : features) assert(f.calls == 1);
}
'''


def compile_and_run(folder, name, content):
    source = folder / (name + ".cpp")
    source.write_text(content, encoding="utf-8")
    exe = folder / (name + (".exe" if os.name == "nt" else ""))
    compiler = shutil.which("cl") if os.name == "nt" else None
    if compiler:
        command = [compiler, "/nologo", "/EHsc", "/std:c++17", "/UNDEBUG", str(source), "/Fe:" + str(exe)]
    else:
        compiler = shutil.which("g++") or shutil.which("clang++")
        if not compiler:
            raise RuntimeError("Install g++/clang++ or use an MSVC developer shell")
        command = [compiler, "-std=c++17", "-UNDEBUG", str(source), "-o", str(exe)]
    subprocess.run(command, cwd=folder, check=True)
    subprocess.run([str(exe)], cwd=folder, check=True)
    print(name + ": passed", flush=True)


def main():
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        (folder / "stdafx.h").write_text(STUBS, encoding="utf-8")
        (folder / "Tile.h").write_text('#include "stdafx.h"\n', encoding="utf-8")
        for name in ("net.minecraft.h", "net.minecraft.world.level.h", "net.minecraft.world.h"):
            (folder / name).write_text('#include "stdafx.h"\n', encoding="utf-8")
        for name in ("Sponge.cpp", "Sponge.h"):
            shutil.copyfile(ROOT / "Minecraft.World" / name, folder / name)
        compile_and_run(folder, "sponge_regression", SPONGE_TEST)
        source = (ROOT / "Minecraft.World/RandomLevelSource.cpp").read_text(encoding="utf-8")
        start = source.index("void RandomLevelSource::recreateLogicStructuresForChunk(")
        opening = source.index("{", start)
        depth = 1
        end = opening + 1
        while depth:
            depth += (source[end] == "{") - (source[end] == "}")
            end += 1
        compile_and_run(folder, "monument_reconstruction", MONUMENT_FIXTURE + source[start:end] + MONUMENT_TEST)


if __name__ == "__main__":
    main()
