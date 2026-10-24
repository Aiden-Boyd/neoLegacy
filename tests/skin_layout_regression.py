"""Compile production skin-height/binding code and layout helper without a GPU."""
from pathlib import Path
import importlib.util
import re
import shutil
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def method(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth, end = 1, opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'

FIXTURE = r'''
#include <cassert>
#include <string>
#include <map>
#include <unordered_map>
#include "Skins.h"
using namespace std;
struct BufferedImage {
    inline static int live=0;
    int height;
    explicit BufferedImage(int h):height(h){++live;}
    ~BufferedImage(){--live;}
    int getHeight(){return height;}
};
struct MemTexture {
    int id=-1,ticksSinceLastUse=100;
    bool isLoaded=false;
    BufferedImage *loadedImage=nullptr;
};
struct MobSkinMemTextureProcessor {};
struct C4JRender { enum {TEXTURE_FORMAT_RxGyBzAw}; };
struct App {
    bool ready=false;
    bool IsFileInMemoryTextures(const wstring&url){return ready&&url==L"arriving";}
} app;
struct Textures {
    bool MIPMAP=true;
    int reads=0,uploads=0;
    const wchar_t *preLoaded[TN_COUNT]={};
    unordered_map<wstring,MemTexture*>memTextures;
    unordered_map<int,int>fallbackImageHeights;
    map<int,int>heights;
    MemTexture arriving;
    Textures(){for(int i=0;i<TN_COUNT;++i)preLoaded[i]=L"test";}
    BufferedImage *readImage(TEXTURE_NAME id,const wstring&){++reads;return heights.count(id)?new BufferedImage(heights[id]):nullptr;}
    MemTexture *addMemTexture(const wstring&url,MobSkinMemTextureProcessor*p){delete p;memTextures[url]=&arriving;return &arriving;}
    int getTexture(BufferedImage*,int,bool){++uploads;return 9001;}
    void loadTexture(BufferedImage*,int){++uploads;}
    int loadTexture(int id){return 10000+id*17;}
    int getHeight(const wstring&,int);
    int loadMemTexture(const wstring&,int);
};
'''

TEST = r'''
int main(){
    Textures t;
    for(int i=0;i<18;++i)t.heights[TN_MOB_CHAR+i]=i%2?64:32;
    // All eighteen default skins share an empty URL. Height queries must not
    // turn the first fallback into a custom skin used for every other preview.
    for(const wstring url:{L"",L"default",L"missing-dlc"}) {
        for(int pass=0;pass<2;++pass)for(int i=0;i<18;++i){
            int tex=TN_MOB_CHAR+i;
            assert(t.getHeight(url,tex)==(i%2?64:32));
            assert(t.loadMemTexture(url,tex)==10000+tex*17);
            assert(!SkinLayout::isSlimDefaultSkin(L"dlcskin-123",tex));
            assert(SkinLayout::isSlimDefaultSkin(L"",tex)==(i>=9));
            assert(SkinLayout::isSlimDefaultSkin(L"default",tex)==(i>=9));
            assert(SkinLayout::isSlimDefaultSkin(L"defskin0000000A.png",tex)==(i>=9));
        }
    }
    assert(t.memTextures.empty()&&t.uploads==0&&t.reads==18&&BufferedImage::live==0);
    assert(!SkinLayout::isSlimDefaultSkin(L"",-1));
    assert(!SkinLayout::isSlimDefaultSkin(L"",TN_COUNT+12345));
    assert(!SkinLayout::isSlimDefaultSkin(L"ugcskin00000001.png",TN_MOB_CHAR9));
    int reads=t.reads;
    assert(t.getHeight(L"",-1)==32&&t.getHeight(L"",TN_COUNT)==32&&t.reads==reads);
    // An unavailable custom skin must not be masked by a cached fallback when
    // its real pixels arrive; metadata inspection performs no GPU upload.
    assert(t.getHeight(L"arriving",TN_MOB_CHAR)==32);
    BufferedImage modern(64);t.arriving.loadedImage=&modern;app.ready=true;
    assert(t.getHeight(L"arriving",TN_MOB_CHAR)==64&&t.uploads==0);
    assert(t.loadMemTexture(L"arriving",TN_MOB_CHAR)==9001&&t.uploads==1);
    assert(t.arriving.ticksSinceLastUse==0);
    MemTexture pending;t.memTextures[L"pending"]=&pending;
    assert(t.getHeight(L"pending",TN_MOB_CHAR)==32);
    // The production reload invalidation must let a replacement pack change
    // a default skin's dimensions instead of retaining the previous layout.
    t.heights[TN_MOB_CHAR]=64;
    RELOAD_INVALIDATION
    assert(t.getHeight(L"",TN_MOB_CHAR)==64);
}
'''

def main():
    spec = importlib.util.spec_from_file_location('base', ROOT/'tests/base_stability_regression.py')
    base = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(base)
    textures = (ROOT/'Minecraft.Client/Textures.cpp').read_text()
    reload = method(textures, 'void Textures::reloadAll()')
    assert reload.index('fallbackImageHeights.clear();') < reload.index('loadIndexedTextures();')
    for file,count in [('PlayerRenderer.cpp',3),('LivingEntityRenderer.cpp',3),('Common/UI/UIControl_PlayerSkinPreview.cpp',1)]:
        code=(ROOT/'Minecraft.Client'/file).read_text()
        assert code.count('SkinLayout::isSlimDefaultSkin(')==count
        assert 'slim[skinId]' not in code
        assert 'getPlayerDefaultSkin() + 35' not in code
        assert 'm_backupTexture) - 37' not in code
    with tempfile.TemporaryDirectory() as temp:
        folder=Path(temp)
        shutil.copyfile(ROOT/'Minecraft.Client/Skins.h',folder/'Skins.h')
        # Use the actual enum ordering; GPU IDs deliberately have no relation.
        header=(ROOT/'Minecraft.Client/Textures.h').read_text()
        enum=re.search(r'typedef enum _TEXTURE_NAME\s*\{.*?\}\s*TEXTURE_NAME;',header,re.S)
        assert enum
        (folder/'Textures.h').write_text('#pragma once\n'+enum.group())
        source=FIXTURE+method(textures,'int Textures::getHeight(const wstring& url, int backup)')
        source+=method(textures,'int Textures::loadMemTexture(const wstring& url, int backup)')
        source+=TEST.replace('RELOAD_INVALIDATION','t.fallbackImageHeights.clear();')
        base.compile_and_run(folder,'skin_layout',source)
    print('Skin fallback identity, 18 layouts, real custom arrival, reload invalidation and binding: passed')

if __name__=='__main__':
    main()
