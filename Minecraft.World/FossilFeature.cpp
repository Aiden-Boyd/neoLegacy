#include "stdafx.h"
#include "FossilFeature.h"
#include "FossilTemplates.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
namespace {
bool natural(int id) {return id==0 || id==1 || id==2 || id==3 || id==12 || id==13 || id==16 || id==24 || id==216;}
void rotate(int &x,int &z,int sx,int sz,int rotation)
{
    int oldX=x,oldZ=z;
    switch(rotation){case 1:x=sz-1-oldZ;z=oldX;break;case 2:x=sx-1-oldX;z=sz-1-oldZ;break;case 3:x=oldZ;z=sx-1-oldX;break;}
}
}
void FossilFeature::decorate(Level *level,int xo,int zo)
{
    if(level->isClientSide)return;
    // Independent stream preserves every existing tree/ore/well RNG draw.
    Random random(level->getSeed() ^ int64_t(xo/16)*341873128712LL ^ int64_t(zo/16)*132897987541LL ^ 987234911LL);
    if(random.nextInt(64)!=0)return;
    int rotation=random.nextInt(4),variant=random.nextInt(8);
    place(level,&random,xo,zo,variant,rotation);
}
bool FossilFeature::place(Level *level,Random *random,int xo,int zo,int variant,int rotation)
{
    if(level->isClientSide || variant<0 || variant>=8 || rotation<0 || rotation>=4)return false;
    const auto &bone=FossilTemplates::templates[variant*2];
    int sx=(rotation&1)?bone.sz:bone.sx,sz=(rotation&1)?bone.sx:bone.sz;
    int ox=xo+random->nextInt(16-sx),oz=zo+random->nextInt(16-sz),height=255;
    for(int x=0;x<sx;++x)for(int z=0;z<sz;++z) {
        if(!level->hasChunkAt(ox+x,0,oz+z))return false;
        height=(std::min)(height,level->getHeightmap(ox+x,oz+z));
    }
    int oy=(std::max)(10,height-15-random->nextInt(10));bool placed=false;
    for(int pass=0;pass<2;++pass) {
        const auto &t=FossilTemplates::templates[variant*2+pass];
        for(int i=0;i<t.count;++i) {
            const auto &cell=t.cells[i];
            if(random->nextFloat()>(pass==0?.9f:.1f))continue;
            int x=cell.x,z=cell.z;rotate(x,z,t.sx,t.sz,rotation);x+=ox;z+=oz;int y=oy+cell.y;
            if(x<xo || x>=xo+16 || z<zo || z>=zo+16 || y<=0 || y>=Level::genDepth || !level->hasChunkAt(x,y,z))continue;
            if(!natural(level->getTile(x,y,z)) || level->getTileEntity(x,y,z))continue;
            int data=cell.data;if((rotation&1) && data!=0)data=data==4?8:4;
            if(level->setTileAndData(x,y,z,cell.id,data,Tile::UPDATE_CLIENTS))placed=true;
        }
    }
    return placed;
}
