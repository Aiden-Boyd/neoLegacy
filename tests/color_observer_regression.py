from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from flight_inventory_regression import method
ROOT=Path(__file__).resolve().parents[1]
def clean(path):return '\n'.join(l for l in (ROOT/path).read_text().splitlines()if not l.startswith('#include')and not l.startswith('#pragma'))
TEST=r'''
int main(){
 Tile stone(1,Material::stone);ConcreteTile solid(251);ConcretePowderTile powder(252);ObserverTile observer(218);IconRegister icons;solid.registerIcons(&icons);powder.registerIcons(&icons);observer.registerIcons(&icons);
 for(int c=0;c<16;++c){assert(solid.getSpawnResourcesAuxValue(c)==c&&powder.getSpawnResourcesAuxValue(c)==c);assert(solid.getTexture(1,c)->name.find(L"concrete_")==0);assert(solid.getTexture(1,c)!=solid.getTexture(1,(c+1)&15));}
 for(int c=0;c<16;++c)for(int face=0;face<6;++face){Level l;l.blocks[{0,64,0}]=252;l.data[{0,64,0}]=c;l.blocks[{Facing::STEP_X[face],64+Facing::STEP_Y[face],Facing::STEP_Z[face]}]=9;powder.onPlace(&l,0,64,0);assert(l.getTile(0,64,0)==(face==0?252:251));assert(l.getData(0,64,0)==c);}
 {Level l;l.blocks[{0,64,0}]=252;l.blocks[{1,64,0}]=10;powder.neighborChanged(&l,0,64,0,10);assert(l.getTile(0,64,0)==252);l.blocks[{1,64,0}]=9;l.isClientSide=true;powder.onPlace(&l,0,64,0);assert(l.getTile(0,64,0)==252);}
 for(int c=0;c<16;++c){Level l;l.blocks[{0,64,0}]=252;l.data[{0,64,0}]=c;powder.tick(&l,0,64,0,nullptr);assert(l.entities.size()==1&&l.getTile(0,64,0)==252);auto f=dynamic_pointer_cast<FallingTile>(l.entities[0]);assert(f&&f->data==c);}
 for(int c=0;c<16;++c){Level l;l.blocks[{0,64,0}]=252;l.data[{0,64,0}]=c;l.blocks[{0,59,0}]=1;HeavyTile::instaFall=true;powder.tick(&l,0,64,0,nullptr);assert(l.getTile(0,64,0)==0&&l.getTile(0,60,0)==252&&l.getData(0,60,0)==c);HeavyTile::instaFall=false;}
 {Level l;l.blocks[{0,64,0}]=252;l.data[{0,64,0}]=12;l.blocks[{0,60,0}]=9;HeavyTile::instaFall=true;powder.tick(&l,0,64,0,nullptr);assert(l.getTile(0,60,0)==251&&l.getData(0,60,0)==12);HeavyTile::instaFall=false;}
 for(int c=0;c<16;++c){Level l;l.blocks[{-1,61,2}]=8;FallingTile f(&l,-.5,64.5,2.5,252,c);f.y=59.5;assert(ConcretePowderTile::hardenFalling(&f));assert(f.removed&&l.getTile(-1,61,2)==251&&l.getData(-1,61,2)==c);}
 {Level l;l.blocks[{0,61,0}]=9;l.failWrite=true;FallingTile f(&l,.5,64.5,.5,252,3);f.y=59.5;assert(!ConcretePowderTile::hardenFalling(&f)&&!f.removed);l.failWrite=false;l.isClientSide=true;assert(!ConcretePowderTile::hardenFalling(&f));}
 for(int c=0;c<16;++c){GlazedTerracottaTile glazed(235+c,c);glazed.registerIcons(&icons);assert(glazed.getPistonPushReaction()==3&&glazed.getSpawnResourcesAuxValue(3)==0);for(int q=0;q<4;++q){Level l;auto p=make_shared<LivingEntity>();p->yRot=float(q*90);glazed.setPlacedBy(&l,0,64,0,p,nullptr);assert(l.getData(0,64,0)==((q+2)&3));for(int face=0;face<6;++face)assert(glazed.getTexture(face,q)->name.find(L"_r"+to_wstring(GlazedTerracottaTile::textureRotation(face,q)))!=wstring::npos);}}
 // Every facing: only an actual state write at the watched face starts a two-tick pulse.
 for(int facing=0;facing<6;++facing){Level l;l.blocks[{0,64,0}]=218;l.data[{0,64,0}]=facing;int fx=Facing::STEP_X[facing],fy=64+Facing::STEP_Y[facing],fz=Facing::STEP_Z[facing];assert(l.setTileAndData(fx,fy,fz,1,0,2));assert(l.pending.size()==1);l.advance(1);assert(!(l.getData(0,64,0)&8));l.setData(fx,fy,fz,1,2);assert(l.pending.size()==1);l.advance(1);assert(l.getData(0,64,0)&8);for(int side=0;side<6;++side)assert(observer.getSignal(&l,0,64,0,side)==(side==facing?15:0));l.advance(2);assert(!(l.getData(0,64,0)&8)&&l.pending.empty()&&l.notices==4);assert(!l.setData(fx,fy,fz,1,2));assert(l.pending.empty());l.setData(fx,fy,fz,2,2);assert(l.pending.size()==1);}
 for(int flags:{0,4,12,18}){Level l;l.blocks[{0,64,0}]=218;l.data[{0,64,0}]=5;l.setTileAndData(1,64,0,1,0,flags);assert(l.pending.empty());}
 {Level l;l.blocks[{0,64,0}]=218;l.data[{0,64,0}]=5;l.isClientSide=true;l.setTileAndData(1,64,0,1,0,3);assert(l.pending.empty());}
 {Level l;l.blocks[{0,64,0}]=218;l.data[{0,64,0}]=5;for(int f=0;f<5;++f)l.setTileAndData(Facing::STEP_X[f],64+Facing::STEP_Y[f],Facing::STEP_Z[f],1,0,3);assert(l.pending.empty());}
 {Level l;l.setTileAndData(0,64,0,218,5,3);assert(l.pending.size()==1);l.advance(2);assert(l.getData(0,64,0)&8);int before=l.notices;l.setTileAndData(0,64,0,0,0,3);assert(l.notices>before);l.advance(2);assert(l.getTile(0,64,0)==0);}
 {Level l;l.blocks[{0,64,0}]=218;l.data[{0,64,0}]=5;observer.onPlace(&l,0,64,0);l.advance(2);Level loaded;loaded.blocks=l.blocks;loaded.data=l.data;loaded.pending=l.pending;loaded.clock=l.clock;loaded.advance(2);assert(!(loaded.getData(0,64,0)&8));}
}
'''
def main():
 source=(ROOT/'tests/color_observer_fixture.h').read_text()+'\n'+clean('Minecraft.World/ColorUpdateTiles.h')+'\n'+clean('Minecraft.World/ObserverTile.h')+'\n'
 source+=method('Minecraft.World/Level.cpp','bool Level::setTileAndData(')+'\n'+method('Minecraft.World/Level.cpp','bool Level::setData(')+'\n'
 source+=clean('Minecraft.World/ColorUpdateTiles.cpp')+'\n'+clean('Minecraft.World/ObserverTile.cpp')
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'concrete_glazed_observer',source+TEST)
 print('All colors, water/falling hardening, glazed orientation, native state hooks and observer pulse timing: passed')
if __name__=='__main__':main()
