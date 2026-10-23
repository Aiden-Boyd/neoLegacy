#pragma once
#include "SimplexNoise.h"
#include <cmath>
#include <cstdint>

// Density inputs use the End generator's eight-block interpolation grid.
// Noise is immutable during sampling, so chunk generation order cannot change terrain.
class EndIslandTerrain
{
public:
    static bool isOuterChunk(int x,int z) {
        return int64_t(x)*x+int64_t(z)*z>4096;
    }
    static float mainIslandHeight(int gridX,int gridZ) {
        float x=float(gridX),z=float(gridZ);
        float height=100.f-std::sqrt(x*x+z*z)*8.f;
        if(height>80) height=80;
        if(height<-100) height=-100;
        return height;
    }
    static float height(SimplexNoise *noise,int gridX,int gridZ) {
        float result=mainIslandHeight(gridX,gridZ);
        // Floor division matters at negative odd grid coordinates: adjacent chunks
        // must sample exactly the same lattice points at their shared boundary.
        int chunkX=gridX/2-(gridX<0&&gridX%2!=0);
        int chunkZ=gridZ/2-(gridZ<0&&gridZ%2!=0);
        int subX=gridX-chunkX*2,subZ=gridZ-chunkZ*2;
        int farX=(chunkX<0?-chunkX:chunkX)+12,farZ=(chunkZ<0?-chunkZ:chunkZ)+12;
        if(int64_t(farX)*farX+int64_t(farZ)*farZ<=4096) return result;
        for(int dx=-12;dx<=12;++dx) for(int dz=-12;dz<=12;++dz) {
            int cx=chunkX+dx,cz=chunkZ+dz;
            if(!isOuterChunk(cx,cz)||noise->getValue(cx,cz)>=-.9) continue;
            int ax=cx<0?-cx:cx,az=cz<0?-cz:cz;
            float slope=float((int64_t(ax)*3439+int64_t(az)*147)%13+9);
            float x=float(subX-dx*2),z=float(subZ-dz*2);
            float candidate=100.f-std::sqrt(x*x+z*z)*slope;
            if(candidate>80) candidate=80;
            if(candidate<-100) candidate=-100;
            if(candidate>result) result=candidate;
        }
        return result;
    }
};
