"""Differential QA for the bounded absorption workspace; uses production Sponge.cpp.

Run with CXXFLAGS='-O2 -fsanitize=address,undefined -fno-omit-frame-pointer'
on Linux for bounds/undefined-behavior checks. The reference retains the previous
heap-backed traversal, including neighbor order, depth and count overshoot.
"""
from pathlib import Path
import tempfile
import base_stability_regression as fixture

TEST = r'''
#include <new>
#include <cstdio>
static size_t allocations = 0;
void* operator new(size_t n) {
    ++allocations;
    if (void* p = malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { free(p); }
void operator delete(void* p, size_t) noexcept { free(p); }
#include "Sponge.cpp"
struct TestSponge : Sponge {
    TestSponge() : Sponge(19) {}
    using Sponge::onPlace;
};

bool reference(Level* level, int x, int y, int z) {
    std::queue<std::tuple<int,int,int,int>> queue;
    std::vector<Position> absorbed;
    queue.push({x,y,z,0});
    int count = 0;
    int dx[] = {1,-1,0,0,0,0}, dy[] = {0,0,1,-1,0,0}, dz[] = {0,0,0,0,1,-1};
    while (!queue.empty()) {
        auto [bx,by,bz,depth] = queue.front(); queue.pop();
        for (int i=0; i<6; ++i) {
            int nx=bx+dx[i], ny=by+dy[i], nz=bz+dz[i];
            if (level->getMaterial(nx,ny,nz) == Material::water) {
                Position position = {nx,ny,nz};
                if (std::find(absorbed.begin(),absorbed.end(),position) != absorbed.end()) continue;
                absorbed.push_back(position); ++count;
                if (depth < 6) queue.push({nx,ny,nz,depth+1});
            }
        }
        if (count > 64) break;
    }
    if (absorbed.empty()) return false;
    if (!level->setTileAndData(x,y,z,Tile::sponge_Id,1,Tile::UPDATE_CLIENTS)) return false;
    for (auto [ax,ay,az] : absorbed)
        if (level->getMaterial(ax,ay,az) == Material::water) level->removeTile(ax,ay,az);
    return true;
}

int main() {
    TestSponge sponge;
    unsigned state=1234567;
    int maximum=0;
    for (int scenario=0; scenario<1000; ++scenario) {
        Level original, candidate;
        int origin=(scenario%3-1)*10000;
        original.sponge(origin,0);
        for (int x=-8;x<=8;++x) for (int y=-8;y<=8;++y) for (int z=-8;z<=8;++z) {
            if (x==0 && y==0 && z==0) continue;
            state=state*1664525u+1013904223u;
            // All air, dense water, isolated pockets, and varied connectivity.
            if (scenario%10==1 || (scenario%10>1 && state%100 < unsigned(scenario%100)))
                original.water(origin+x,y,z);
        }
        original.failWrites=scenario%17==0;
        candidate=original;
        std::vector<Position> beforeOrder, afterOrder;
        original.notify=[&] {
            assert(original.getData(origin,0,0)==1);
            // Capture the exact state at every callback, including removal order.
            for (const auto& entry : original.blocks)
                if (entry.second==0) beforeOrder.push_back(entry.first);
        };
        candidate.notify=[&] {
            assert(candidate.getData(origin,0,0)==1);
            sponge.onPlace(&candidate,origin,0,0); // must not reenter
            for (const auto& entry : candidate.blocks)
                if (entry.second==0) afterOrder.push_back(entry.first);
        };
        reference(&original,origin,0,0);
        sponge.onPlace(&candidate,origin,0,0);
        assert(original.blocks==candidate.blocks);
        assert(original.data==candidate.data);
        assert(original.removals==candidate.removals);
        assert(beforeOrder==afterOrder);
        assert(candidate.removals<=70);
        maximum=std::max(maximum,candidate.removals);
    }
    assert(maximum>64); // exercise the legacy end-of-node count overshoot

    Level line;
    line.sponge(0,0);
    for (int x=1;x<=9;++x) line.water(x,0,0);
    sponge.onPlace(&line,0,0,0);
    assert(line.removals==7);
    assert(line.getMaterial(8,0,0)==Material::water);

    Level empty;
    empty.sponge(0,0);
    sponge.onPlace(&empty,0,0,0); // warm the fixture's sparse map
    size_t start=allocations;
    for (int i=0;i<10000;++i) sponge.onPlace(&empty,0,0,0);
    size_t candidateAllocations=allocations-start;
    assert(candidateAllocations==0);
    start=allocations;
    for (int i=0;i<10000;++i) reference(&empty,0,0,0);
    size_t referenceAllocations=allocations-start;
    assert(referenceAllocations>0);
    std::printf("1000 differential worlds passed; max removed=%d; empty attempts: baseline=%zu allocations, candidate=%zu\n",
        maximum,referenceAllocations,candidateAllocations);
}
'''


def main():
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        # Reuse the same deterministic engine definitions as the existing QA.
        (folder / "stdafx.h").write_text(fixture.STUBS, encoding="utf-8")
        for name in ("Tile.h", "net.minecraft.h", "net.minecraft.world.level.h", "net.minecraft.world.h"):
            (folder / name).write_text('#include "stdafx.h"\n', encoding="utf-8")
        for name in ("Sponge.cpp", "Sponge.h"):
            (folder / name).write_bytes((fixture.ROOT / "Minecraft.World" / name).read_bytes())
        # The allocation instrumentation needs standard declarations before new.
        fixture.compile_and_run(folder, "sponge_workspace", '#include "stdafx.h"\n' + TEST)


if __name__ == "__main__":
    main()
