from pathlib import Path
import hashlib,json,tempfile
from base_stability_regression import compile_and_run
from content_recipe_regression import STUB
from mending_regression import method
from content_assets_regression import png
ROOT=Path(__file__).resolve().parents[1]
def main():
 stub=STUB.replace('*bowl;','*bowl,*endCrystal,*eyeOfEnder,*ghastTear;').replace('*boneBlock;','*boneBlock,*glass;')
 source=(ROOT/'Minecraft.World/Recipes.cpp').read_text();start=source.index('    addShapedRecipy(new ItemInstance(Item::endCrystal');end=source.index('\n',start)
 registration='void Recipes::registerContent(){'+source[start:end]+'}\n'
 test=r'''int main(){Item end{426},eye{381},tear{370};Tile glass{20};Item::endCrystal=&end;Item::eyeOfEnder=&eye;Item::ghastTear=&tear;Tile::glass=&glass;Recipes r;r.registerContent();assert(r.entries.size()==1);auto recipe=dynamic_cast<ShapedRecipy*>(r.entries[0]);assert(recipe&&recipe->width==3&&recipe->height==3&&recipe->result->id==426&&recipe->result->count==1);int ids[9]={20,20,20,20,381,20,20,370,20};for(int i=0;i<9;++i)assert(recipe->ingredients[i]&&recipe->ingredients[i]->id==ids[i]&&recipe->ingredients[i]->count==1);}'''
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'end_crystal_native_recipe',stub+registration+method('Recipes.cpp','ShapedRecipy *Recipes::addShapedRecipy(ItemInstance *result, ...')+test)
 atlas=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res';width,height,pixels=png(atlas/'items.png');size=width//16
 crop=b''.join(pixels[(y*width+7*size)*4:(y*width+8*size)*4]for y in range(13*size,14*size))
 assert len(set(crop))>8
 record=json.loads((ROOT/'tests/content_atlas_hashes.json').read_text())['items.png'];assert [13,7,hashlib.sha256(crop).hexdigest()]in record['cells']
 assert 'ADD_ICON(13, 7, L"end_crystal")'in(ROOT/'Minecraft.Client/PreStitchedTextureMap.cpp').read_text()
 item=(ROOT/'Minecraft.World/Item.cpp').read_text();assert 'new EndCrystalItem(endCrystal_Id-256)'in item
 assert 'ITEM(Item::endCrystal_Id)'in(ROOT/'Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp').read_text()
 assert 'EndFight::tick(this)'in(ROOT/'Minecraft.Client/ServerLevel.cpp').read_text()
 data=(ROOT/'Minecraft.World/LevelData.cpp').read_text();assert 'endFight.load(tag)'in data and 'endFight.save(tag)'in data and 'endFight = copy->getEndFight()'in data
 assert 'return wrapped->getEndFight()'in(ROOT/'Minecraft.World/DerivedLevelData.cpp').read_text()
 renderer=(ROOT/'Minecraft.Client/EnderCrystalRenderer.cpp').read_text();assert 'hasBeamTarget()'in renderer and 'TN_MOB_ENDERDRAGON_BEAM'in renderer
 assert 'shouldShowBottom()'in(ROOT/'Minecraft.Client/EnderCrystalModel.cpp').read_text()
 print('Crystal recipe, UI, atlas, saved world state, host tick and renderer integration: passed')
if __name__=='__main__':main()
