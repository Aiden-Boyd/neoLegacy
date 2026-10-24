#pragma once
// Only cells introduced by our cumulative content batches; no old pack cells.
namespace AddedTerrainFallback {
static const int cells[][2]={{23,5},{23,6},{23,7},{23,8},{23,9},{23,10},{24,5},{24,6},{24,7},{24,8},{24,9},{24,10},{24,11},{24,12},{24,13},{24,14},{24,15},{25,4},{25,5},{25,6},{25,7},{25,8},{25,9},{25,10},{25,11},{25,12},{25,13},{25,14},{25,15},{26,0},{26,1},{26,2},{26,3},{26,4},{26,5},{26,6},{26,7},{26,8},{26,9},{26,10},{26,11},{26,12},{26,13},{26,14},{26,15},{27,0},{27,1},{27,2},{27,3},{27,4},{27,5},{27,6},{27,7},{27,8},{27,9},{27,10},{27,11},{27,12},{27,13},{27,14},{27,15},{28,0},{28,1},{28,2},{28,3},{28,4},{28,5},{28,6},{28,7},{28,8},{28,9},{28,10},{28,11},{28,12},{28,13},{28,14},{28,15},{29,0},{29,1},{29,2},{29,3},{29,4},{29,5},{29,6},{29,7},{29,8},{29,9},{29,10},{29,11},{29,12},{29,13},{29,14},{29,15},{30,0},{30,1},{30,2},{30,3},{30,4},{30,5},{30,6},{30,7},{30,8},{30,9},{30,10},{30,11},{30,12},{30,13},{30,14},{30,15},{31,0},{31,1},{31,2},{31,3},{31,4},{31,5},{31,6},{31,7},{31,8},{31,9},{31,10},{31,11},{31,12},{31,13},{31,14}};
template<class Image> bool compatible(Image *image) {
    return image&&image->getWidth()>=16&&(image->getWidth()&(image->getWidth()-1))==0&&image->getWidth()%16==0&&image->getHeight()==image->getWidth()*2&&image->getData(0);
}
template<class Image> bool empty(Image *image,int row,int col) {
    int size=image->getWidth()/16;auto pixels=image->getData(0);int width=image->getWidth();
    for(int y=row*size;y<(row+1)*size;++y)for(int x=col*size;x<(col+1)*size;++x)
        if((static_cast<unsigned int>(pixels[y*width+x])>>24)!=0)return false;
    return true;
}
template<class Image> bool needed(Image *image) {
    if(!compatible(image))return false;
    for(auto &cell:cells)if(empty(image,cell[0],cell[1]))return true;
    return false;
}
template<class Image> int apply(Image *image,Image *fallback) {
    if(!compatible(image)||!compatible(fallback))return 0;
    int restored=0;
    for(auto &cell:cells) {
        int row=cell[0],col=cell[1];if(!empty(image,row,col))continue;
        for(int level=0;level<10;++level) {
            int width=image->getWidth()>>level,size=width/16;
            if(size<1)break;auto dst=image->getData(level);if(!dst)continue;
            int sourceLevel=level,sw=fallback->getWidth()>>level;
            if(sw<16||!fallback->getData(level)){sourceLevel=0;sw=fallback->getWidth();}
            int ss=sw/16;auto src=fallback->getData(sourceLevel);
            for(int y=0;y<size;++y)for(int x=0;x<size;++x)
                dst[(row*size+y)*width+col*size+x]=src[(row*ss+y*ss/size)*sw+col*ss+x*ss/size];
        }
        ++restored;
    }
    return restored;
}
}
