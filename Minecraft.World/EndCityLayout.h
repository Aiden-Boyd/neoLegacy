#pragma once
#include "EndCityTemplates.h"
#include "Random.h"
#include <vector>
#include <algorithm>

// Pure planning: no world access and no shared random stream. A city is rebuilt
// identically regardless of which of its population chunks is requested first.
namespace EndCityLayout {
struct Point {int x,y,z;};
inline Point rotate(Point p,int r) {switch(r&3){case 1:return {-p.z,p.y,p.x};case 2:return {-p.x,p.y,-p.z};case 3:return {p.z,p.y,-p.x};default:return p;}}
struct Part {
    const EndCityTemplates::Room *room;Point pos;int rotation;bool overwrite;
    Point world(Point p) const {p=rotate(p,rotation);return {p.x+pos.x,p.y+pos.y,p.z+pos.z};}
    void bounds(int &x0,int &y0,int &z0,int &x1,int &y1,int &z1) const {
        Point a=world({0,0,0}),b=world({room->sx-1,room->sy-1,room->sz-1});
        x0=(std::min)(a.x,b.x);x1=(std::max)(a.x,b.x);y0=a.y;y1=b.y;z0=(std::min)(a.z,b.z);z1=(std::max)(a.z,b.z);
    }
};
inline unsigned char rotateData(int id,int data,int rotation) {
    if(id==202&&rotation%2&&(data==4||data==8)) return data==4?8:4;
    if(id==203) {int cycle[4]={2,3,1,0};for(int i=0;i<rotation;++i)data=(data&4)|cycle[data&3];return data;}
    if(id==54||id==65||id==130||id==144||id==177||id==198) {
        const int clockwise[6]={0,1,5,4,2,3};for(int i=0;i<rotation;++i) data=clockwise[data];return data;
    }
    return data;
}
inline int floorDiv(int n,int d) {int q=n/d;return n<0&&n%d?q-1:q;}
inline void site(int64_t seed,int rx,int rz,int &cx,int &cz) {
    uint64_t mixed=uint64_t(seed)+uint64_t(int64_t(rx)*341873128712LL)+uint64_t(int64_t(rz)*132897987541LL)+10387313ULL;
    Random random(static_cast<int64_t>(mixed));
    cx=rx*20+(random.nextInt(9)+random.nextInt(9))/2;
    cz=rz*20+(random.nextInt(9)+random.nextInt(9))/2;
}
struct Plan {
    std::vector<Part> parts;
    bool hasShip=false;
    int add(const char *name,Point p,int rotation,bool overwrite=true) {
        auto room=EndCityTemplates::find(name);if(!room) return -1;
        parts.push_back({room,p,rotation&3,overwrite});return int(parts.size())-1;
    }
    int child(int parent,const char *name,Point offset,int rotation=0,bool overwrite=true) {
        const auto origin=parts[parent];return add(name,origin.world(offset),(origin.rotation+rotation)&3,overwrite);
    }
    int house(Point p,int rotation,Random &random,bool main=false) {
        int base=add("base_floor",p,rotation),last=base,stories=main?2:random.nextInt(3);
        if(stories>0) last=child(last,main?"second_floor":"second_floor_2",{-1,0,-1},0,false);
        if(stories>1) last=child(last,main?"third_floor":random.nextInt(2)?"third_floor_b":"third_floor_c",{-1,4,-1},0,false);
        return child(last,stories==0?"base_roof":stories==1?"second_roof":"third_roof",{-1,stories==0?4:8,-1});
    }
    void tower(int roof,Random &random,bool fat) {
        if(fat) {
            int last=child(roof,"fat_tower_base",{0,-3,0});
            for(int n=0,count=1+random.nextInt(3);n<count;++n) last=child(last,"fat_tower_middle",{0,n==0?4:8,0});
            child(last,"fat_tower_top",{-2,8,-2});return;
        }
        int last=child(roof,"tower_base",{3,-3,3});
        int count=1+random.nextInt(3);
        for(int i=0;i<count;++i) last=child(last,"tower_piece",{0,i==0?7:4,0});
        child(last,"tower_top",{-1,4,-1});
    }
    Plan(int64_t seed,int cx,int cz,int y,int rotation) {
        Random random(seed^int64_t(cx)*341873128712LL^int64_t(cz)*132897987541LL);
        int roof=house({cx*16+8,y,cz*16+8},rotation,random,true);
        int towerBase=child(roof,"tower_base",{3,-3,3}),last=towerBase;
        for(int i=0,n=1+random.nextInt(3);i<n;++i) last=child(last,"tower_piece",{0,i==0?7:4,0});
        child(last,"tower_top",{-1,4,-1});
        int used=0,branches=1+random.nextInt(3);
        for(int b=0;b<branches;++b) {
            int direction=random.nextInt(4);while(used&(1<<direction)) direction=(direction+1)&3;used|=1<<direction;
            // Rotate about the tower center before moving out through its side.
            Point center=parts[last].world({3,-1,3}),offset=rotate({-2,0,4},(rotation+direction)&3);
            Point start={center.x+offset.x,center.y,center.z+offset.z};
            int r=(rotation+direction)&3,bridge=add("bridge_end",start,r),length=0,rise=0;
            int segments=2+random.nextInt(5);
            for(int n=0;n<segments;++n) {
                int choice=random.nextInt(4);if(choice<2&&start.y+rise-4<y+4) choice=2;const char *piece=choice==0?"bridge_steep_stairs":choice==1?"bridge_gentle_stairs":"bridge_piece";
                bridge=add(piece,Part{nullptr,start,r,false}.world({0,rise-(choice<2?4:0),length}),r);
                length+=choice==1?8:4;if(choice<2) rise-=4;
            }
            int end=add("bridge_end",Part{nullptr,start,r,false}.world({0,rise,length}),r);
            Point housePos=parts[end].world({7,1,10});int otherRoof=house(housePos,(r+2)&3,random);
            tower(otherRoof,random,random.nextInt(3)==0);
            if(!hasShip&&random.nextInt(3)==0) {child(end,"ship",{-8,2,22});hasShip=true;}
        }
    }
    bool valid(int edge,int height) const {
        for(const auto &p:parts) {int x0,y0,z0,x1,y1,z1;p.bounds(x0,y0,z0,x1,y1,z1);if(x0< -edge+2||z0< -edge+2||x1>=edge-2||z1>=edge-2||y0<1||y1>=height-2)return false;}return true;
    }
};
}
