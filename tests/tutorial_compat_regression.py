"""Exercise production banner data/entity/item and slime landing code without a GPU."""
from pathlib import Path
import hashlib,json,re,tempfile
from base_stability_regression import compile_and_run
from mending_regression import method
from content_assets_regression import png,verify_assets
from content_recipe_regression import STUB
ROOT=Path(__file__).resolve().parents[1];W=ROOT/'Minecraft.World'
TEST=r'''
int main(){
 EndCityBanner standing(176),wall(177);SlimeBlockTile slime(165);BannerItem item(169);
 assert(!slime.isSolidRender()&&slime.getRenderLayer()==1&&slime.friction==.8f);
 for(int color=0;color<16;++color){
  BannerData a;a.plain(color);a.name=L"Tutorial";a.patterns={{L"cr",1},{L"unknown_mod_pattern",7}};CompoundTag tag;a.save(&tag);BannerData b;b.load(&tag);assert(b.base==color&&b.name==a.name&&b.patterns==a.patterns&&!b.cityStyle);auto c=b;c.patterns[0].second=4;assert(b.patterns[0].second==1);
  auto te=make_shared<EndCityBannerEntity>();te->x=-16;te->y=71;te->z=85;te->banner=b;CompoundTag saved;te->save(&saved);EndCityBannerEntity restored;restored.load(&saved);assert(restored.x==-16&&restored.banner.patterns==b.patterns);
  auto clone=dynamic_pointer_cast<EndCityBannerEntity>(te->clone());clone->banner.name=L"Independent";clone->banner.patterns.clear();assert(te->banner.name==L"Tutorial"&&te->banner.patterns.size()==2);
  auto packet=dynamic_pointer_cast<TileEntityDataPacket>(te->getUpdatePacket());assert(packet&&packet->type==6&&packet->tag->getInt(L"Base")==color);auto drop=te->asItem();assert(drop->id==425&&drop->count==1&&drop->aux==color&&drop->name==L"Tutorial");
  Level l;l.blocks[{0,63,0}]=1;auto p=make_shared<Player>();assert(item.useOn(drop,p,&l,0,63,0,1,0,0,0));assert(drop->count==0&&l.getTile(0,64,0)==176);auto placed=dynamic_pointer_cast<EndCityBannerEntity>(l.getTileEntity(0,64,0));assert(placed&&placed->banner.base==color&&placed->banner.patterns==b.patterns&&placed->banner.name==b.name);l.setTileAndUpdate(0,64,0,0);assert(l.drops.size()==1&&l.drops[0]->aux==color);l.entities[{0,64,0}]=placed;standing.onRemove(&l,0,64,0,176,0);assert(l.drops.size()==1);
 }
 // Old city saves still get their green banner/black triangles; imported plain white/red stay plain.
 CompoundTag legacy;BannerData old;old.load(&legacy);assert(old.cityStyle&&old.base==2&&old.patterns.size()==2);for(int c:{15,1}){CompoundTag imported;imported.putInt(L"Base",c);imported.put(L"Patterns",new ListTag<CompoundTag>());old.load(&imported);assert(old.base==c&&!old.cityStyle&&old.patterns.empty());}
 // Standard item NBT may contain Patterns without Base. Aux supplies the color.
 {CompoundTag tag;auto layers=new ListTag<CompoundTag>();auto layer=new CompoundTag();layer->putString(L"Pattern",L"bt");layer->putInt(L"Color",0);layers->add(layer);tag.put(L"Patterns",layers);BannerData b;b.load(&tag,1);assert(b.base==1&&b.patterns.size()==1&&!b.cityStyle);}
 for(int angle=0;angle<16;++angle){Level l;l.blocks[{0,63,0}]=1;auto p=make_shared<Player>();p->yRot=angle*22.5f-180;auto b=make_shared<ItemInstance>(425,1,15);assert(item.useOn(b,p,&l,0,63,0,1,0,0,0));assert(l.getData(0,64,0)==angle);}
 for(int face=2;face<=5;++face){Level l;l.blocks[{0,64,0}]=1;auto p=make_shared<Player>();auto b=make_shared<ItemInstance>(425,1,1);p->abilities.instabuild=true;assert(item.useOn(b,p,&l,0,64,0,face,0,0,0));assert(b->count==1);int x=Facing::STEP_X[face],z=Facing::STEP_Z[face];assert(l.getTile(x,64,z)==177&&l.getData(x,64,z)==face);wall.neighborChanged(&l,x,64,z,1);assert(l.getTile(x,64,z)==177);wall.playerWillDestroy(&l,x,64,z,face,p);l.setTileAndUpdate(x,64,z,0);assert(l.drops.empty());}
 {Level l;l.blocks[{0,63,0}]=1;auto p=make_shared<Player>();auto b=make_shared<ItemInstance>(425,3,15);l.isClientSide=true;assert(item.useOn(b,p,&l,0,63,0,1,0,0,0)&&l.writes==0&&b->count==3);l.isClientSide=false;assert(item.useOn(b,p,&l,0,63,0,1,0,0,0,true)&&l.writes==0);l.fail=true;assert(!item.useOn(b,p,&l,0,63,0,1,0,0,0)&&b->count==3);l.fail=false;p->allowed=false;assert(!item.useOn(b,p,&l,0,63,0,1,0,0,0));p->allowed=true;l.loaded=false;assert(!item.useOn(b,p,&l,0,63,0,1,0,0,0));l.loaded=true;assert(!item.useOn(b,p,&l,0,63,0,0,0,0,0));assert(!standing.mayPlace(&l,5,64,5,1));assert(!wall.mayPlace(&l,5,64,5,1));}
 {Level l;l.blocks[{0,63,0}]=1;auto p=make_shared<Player>();auto b=make_shared<ItemInstance>(425,1,1);assert(item.useOn(b,p,&l,0,63,0,1,0,0,0));l.blocks[{0,63,0}]=0;standing.neighborChanged(&l,0,64,0,1);assert(l.getTile(0,64,0)==0&&l.drops.size()==1);}
 assert(SlimeBlockPhysics::land(-1.5,false,true)==1.5);assert(SlimeBlockPhysics::land(-1.5,false,false)==1.2||abs(SlimeBlockPhysics::land(-1.5,false,false)-1.2)<1e-12);assert(SlimeBlockPhysics::land(-1.5,true,true)==0);assert(SlimeBlockPhysics::land(.5,false,true)==0);assert(SlimeBlockPhysics::drag(0,false)==.4&&SlimeBlockPhysics::drag(.2,false)==1&&SlimeBlockPhysics::drag(0,true)==1);
 {Level l;l.blocks[{0,63,0}]=165;Entity e;e.level=&l;e.fallDistance=24;e.checkFallDamage(-1,true);assert(e.damage==0&&e.fallDistance==0);e.sneak=true;e.fallDistance=24;e.checkFallDamage(-1,true);assert(e.damage==24);e.sneak=false;e.damage=0;l.blocks[{0,63,0}]=1;e.fallDistance=24;e.checkFallDamage(-1,true);assert(e.damage==24);e.fallDistance=0;e.checkFallDamage(-2,false);assert(e.fallDistance==2);}
}
'''
def main():
 with tempfile.TemporaryDirectory()as d:
  p=Path(d);(p/'stdafx.h').write_bytes((ROOT/'tests/tutorial_fixture.h').read_bytes())
  for name in ['BaseEntityTile.h','TileEntity.h','CompoundTag.h','ListTag.h','ItemInstance.h','Item.h','Tile.h','Entity.h','net.minecraft.world.level.h','net.minecraft.world.level.tile.h','net.minecraft.world.entity.player.h','net.minecraft.network.packet.h','Facing.h','Mth.h']:(p/name).write_text('#include "stdafx.h"\n')
  for name in ['BannerData','EndCityBanner','BannerItem']:
   for suffix in ['.h','.cpp']:(p/(name+suffix)).write_bytes((W/(name+suffix)).read_bytes())
  for name in ['SlimeBlockTile.h','SlimeBlockPhysics.h']:(p/name).write_bytes((W/name).read_bytes())
  code='#include "stdafx.h"\n#include "BannerData.cpp"\n#include "EndCityBanner.cpp"\n#include "BannerItem.cpp"\n#include "SlimeBlockTile.h"\n'+method('Entity.cpp','void Entity::checkFallDamage(')+TEST
  compile_and_run(p,'tutorial_compat',code)
 source=(W/'Entity.cpp').read_text();assert 'SlimeBlockPhysics::land(yd,isSneaking(),instanceof(eTYPE_LIVINGENTITY))'in source and 'SlimeBlockPhysics::drag(yd,isSneaking())'in source
 assert 'L"Banner"'in(W/'TileEntity.cpp').read_text()and'L"EndCityBanner"'in(W/'TileEntity.cpp').read_text()
 assert 'TileEntityDataPacket::TYPE_BANNER'in(ROOT/'Minecraft.Client/ClientConnection.cpp').read_text()
 tiles=(W/'Tile.cpp').read_text();assert 'new SlimeBlockTile(165)'in tiles and 'new EndCityBanner(176)'in tiles and 'new EndCityBanner(177)'in tiles
 b=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res';m=json.loads((ROOT/'tests/tutorial_asset_hashes.json').read_text());assert len(m['sha256'])==41
 for fn,digest in m['sha256'].items():assert hashlib.sha256((b/fn).read_bytes()).hexdigest()==digest,fn
 for atlas,v in m['cells'].items():
  w,h,pixels=png(b/(atlas+'.png'));s=w//16;row,col=v['row'],v['col'];crop=b''.join(pixels[(y*w+col*s)*4:(y*w+(col+1)*s)*4]for y in range(row*s,(row+1)*s));assert crop==png(b/('textures/blocks/slime.png'if atlas=='terrain'else'textures/items/banner_icon.png'))[2]
 for fn,digest in m['atlas_preservation'].items():
  w,h,pixels=png(b/fn);v=m['cells']['terrain'if fn.startswith('terrain')else'items'];s=w//16;row,col=v['row'],v['col'];masked=bytearray(pixels)
  for y in range(row*s,(row+1)*s):masked[(y*w+col*s)*4:(y*w+(col+1)*s)*4]=bytes(s*4)
  assert hashlib.sha256(masked).hexdigest()==digest,fn
 verify_recipes()
 verify_assets();print('Tutorial block registration, banner lifecycle/packet data, slime landings and original assets: passed')
def verify_recipes():
 stub=STUB.replace('*bowl;','*bowl,*slimeBall,*stick;').replace('*boneBlock;','*boneBlock,*wool;inline static Tile*tiles[256];')
 source=(W/'Recipes.cpp').read_text();a=source.index('    addShapedRecipy(new ItemInstance(165');b=source.index('    addShapedRecipy(new ItemInstance(Tile::observer',a)
 code=stub+'void Recipes::registerContent(){'+source[a:b]+'}\n'+method('Recipes.cpp','ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...')+method('Recipes.cpp','void Recipes::addShapelessRecipy(ItemInstance *result,...')+method('ShapelessRecipy.cpp','bool ShapelessRecipy::matches')
 code+=r'''int main(){Item ball{341},stick{280};Tile slime{165},wool{35};Item::slimeBall=&ball;Item::stick=&stick;Tile::tiles[165]=&slime;Tile::wool=&wool;Recipes r;r.registerContent();assert(r.entries.size()==18);auto block=dynamic_cast<ShapedRecipy*>(r.entries[0]);assert(block&&block->result->id==165&&block->width==3&&block->height==3);for(auto i:block->ingredients)assert(i&&i->id==341);auto unpack=dynamic_cast<ShapelessRecipy*>(r.entries[1]);assert(unpack&&unpack->result->id==341&&unpack->result->count==9);auto grid=make_shared<CraftingContainer>();grid->cells[0]=make_shared<ItemInstance>(165);assert(unpack->matches(grid,nullptr));for(int dye=0;dye<16;++dye){auto banner=dynamic_cast<ShapedRecipy*>(r.entries[2+dye]);assert(banner&&banner->result->id==425&&banner->result->aux==dye&&banner->width==3&&banner->height==3);for(int i=0;i<6;++i)assert(banner->ingredients[i]->id==35&&banner->ingredients[i]->aux==15-dye);assert(!banner->ingredients[6]&&banner->ingredients[7]->id==280&&!banner->ingredients[8]);}}'''
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'tutorial_recipes',code)
if __name__=='__main__':main()
