#include "stdafx.h"
#include "EndOuterFeatures.h"
#include "EndIslandTerrain.h"
#include "Facing.h"
#include "Random.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include <cmath>

bool EndOuterFeatures::clearNeighbors(Level *level,int x,int y,int z,int exceptFace) {
    for(int face=2;face<6;++face) if(face!=exceptFace &&
        !level->isEmptyTile(x+Facing::STEP_X[face],y,z+Facing::STEP_Z[face])) return false;
    return true;
}
bool EndOuterFeatures::growChorus(Level *level,Random *random,int x,int y,int z,int spread) {
    if(level->isClientSide||spread<1||spread>8||y<=0||y>=Level::maxBuildHeight-1) return false;
    if(level->getTile(x,y-1,z)!=Tile::endStone_Id||!level->isEmptyTile(x,y,z)||!clearNeighbors(level,x,y,z)) return false;
    level->setTileAndData(x,y,z,Tile::chorusPlant_Id,0,Tile::UPDATE_CLIENTS);
    branch(level,random,x,y,z,x,z,spread,0);
    return true;
}
void EndOuterFeatures::branch(Level *level,Random *random,int x,int y,int z,int originX,int originZ,int spread,int depth) {
    int length=random->nextInt(4)+1+(depth==0?1:0),grown=0;
    for(int step=1;step<=length;++step) {
        int yy=y+step;
        if(yy>=Level::maxBuildHeight-1||!level->isEmptyTile(x,yy,z)||!clearNeighbors(level,x,yy,z)) break;
        level->setTileAndData(x,yy,z,Tile::chorusPlant_Id,0,Tile::UPDATE_CLIENTS);grown=step;
    }
    int top=y+grown;bool branched=false;
    // A blocked upward segment terminates here; never tunnel through existing blocks.
    if(grown==length&&depth<4) {
        int attempts=random->nextInt(4)+(depth==0?1:0);
        for(int i=0;i<attempts;++i) {
            int face=2+random->nextInt(4);
            int nx=x+Facing::STEP_X[face],nz=z+Facing::STEP_Z[face];
            if(nx-originX<=-spread||nx-originX>=spread||nz-originZ<=-spread||nz-originZ>=spread) continue;
            if(!level->isEmptyTile(nx,top,nz)||!level->isEmptyTile(nx,top-1,nz)||
               !clearNeighbors(level,nx,top,nz,Facing::getOpposite(face))) continue;
            level->setTileAndData(nx,top,nz,Tile::chorusPlant_Id,0,Tile::UPDATE_CLIENTS);
            branch(level,random,nx,top,nz,originX,originZ,spread,depth+1);branched=true;
        }
    }
    if(!branched) level->setTileAndData(x,top,z,Tile::chorusFlower_Id,5,Tile::UPDATE_CLIENTS);
}
void EndOuterFeatures::placeSmallIsland(Level *level,Random *random,int x,int y,int z) {
    if(level->isClientSide||y<=0||y>=Level::maxBuildHeight) return;
    float radius=float(random->nextInt(3)+4);
    for(int yy=y;radius>.5f&&yy>0;--yy) {
        int bound=int(std::ceil(radius));
        for(int dx=-bound;dx<=bound;++dx) for(int dz=-bound;dz<=bound;++dz) {
            if(dx*dx+dz*dz>(radius+1)*(radius+1)||!level->isEmptyTile(x+dx,yy,z+dz)) continue;
            level->setTileAndData(x+dx,yy,z+dz,Tile::endStone_Id,0,Tile::UPDATE_CLIENTS);
        }
        radius-=random->nextInt(2)+.5f;
    }
}
void EndOuterFeatures::decorate(Level *level,Random *random,int chunkX,int chunkZ,float islandHeight) {
    if(level->isClientSide||!EndIslandTerrain::isOuterChunk(chunkX,chunkZ)) return;
    int xo=chunkX*16,zo=chunkZ*16;
    // Population has the current chunk and its +X/+Z neighbors available.
    // All placement and neighbor reads stay within that 32x32 block area.
    if(islandHeight<-20&&random->nextInt(14)==0) {
        int x=xo+8+random->nextInt(16),z=zo+8+random->nextInt(16),y=55+random->nextInt(16);
        placeSmallIsland(level,random,x,y,z);
        if(random->nextInt(4)==0) {
            x=xo+8+random->nextInt(16);z=zo+8+random->nextInt(16);y=55+random->nextInt(16);
            placeSmallIsland(level,random,x,y,z);
        }
    }
    if(islandHeight<=40) return;
    int attempts=random->nextInt(5);
    for(int i=0;i<attempts;++i) {
        int x=xo+8+random->nextInt(16),z=zo+8+random->nextInt(16);
        int y=level->getHeightmap(x,z);
        if(y>0) growChorus(level,random,x,y,z);
    }
}
