#pragma once
class Level;
class Random;

class EndOuterFeatures
{
public:
    static void decorate(Level*,Random*,int chunkX,int chunkZ,float islandHeight);
    static bool growChorus(Level*,Random*,int x,int y,int z,int spread=8);
    static void placeSmallIsland(Level*,Random*,int x,int y,int z);
private:
    static void branch(Level*,Random*,int x,int y,int z,int originX,int originZ,int spread,int depth);
    static bool clearNeighbors(Level*,int x,int y,int z,int exceptFace=-1);
};
