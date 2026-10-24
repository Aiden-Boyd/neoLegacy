#pragma once
// Preserve legacy IDs 0..13 and their pack-defined colours. New IDs follow 1.12.2.
namespace MapPalette {
inline int dye(int data) {static const int ids[16]={8,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29};return ids[data&15];}
inline int block(int id,int data,int original) {
    if(id==165)return 1;
    if(id==35||id==171||id==251||id==252)return dye(data);
    if(id>=235&&id<=250)return dye(id-235);
    if(id>=219&&id<=234)return dye(id-219);
    if(id==159)return 36+(data&15);
    if(id>=201&&id<=205)return 16; // purpur
    if(id==199||id==200)return 24; // chorus
    if(id==206||id==121||id==216)return 2; // End stone and bone
    if(id==207)return 7;
    if(id==208)return 10;
    if(id==212)return 5;
    if(id==213||id==215)return 35;
    if(id==214)return 28;
    if(id==217||id==209)return 0; // structure void / gateway
    if(id==218)return 11;
    return original>=0&&original<14?original:0;
}
inline int rgb(int id) {
    static const int values[52]={0,8368696,16247203,13092807,16711680,10526975,10987431,31744,16777215,10791096,9923917,7368816,4210943,9402184,16776437,14188339,11685080,6724056,15066419,8375321,15892389,5000268,10066329,5013401,8339378,3361970,6704179,6717235,10040115,1644825,16445005,6085589,4882687,55610,8476209,7340544,13742497,10441252,9787244,7367818,12223780,6780213,10505550,3746083,8874850,5725276,8014168,4996700,4993571,5001770,9321518,2430480};
    return id>=0&&id<52?values[id]:0;
}
inline int brightness(int shade) {return shade==0?180:shade==2?255:shade==3?135:220;}
}
