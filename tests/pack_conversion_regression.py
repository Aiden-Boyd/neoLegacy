"""Exercise the offline atlas converter with synthetic, non-game artwork."""
import importlib.util,io,json,tempfile,zipfile
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('converter',ROOT/'tools/convert_java_pack.py');c=importlib.util.module_from_spec(spec);spec.loader.exec_module(c)
def png(color,height=16):
 b=io.BytesIO();Image.new('RGBA',(16,height),color).save(b,format='PNG');return b.getvalue()
def main():
 with tempfile.TemporaryDirectory() as tmp:
  p=Path(tmp);base=p/'base';base.mkdir()
  for name,height in [('terrain',512),('items',256)]:Image.new('RGBA',(256,height),(17,18,19,255)).save(base/(name+'.png'))
  pack=p/'pack.zip'
  with zipfile.ZipFile(pack,'w') as z:
   z.writestr('pack.mcmeta',json.dumps({'pack':{'pack_format':3,'description':'test'}}))
   z.writestr('assets/minecraft/textures/blocks/stone.png',png((210,30,40,255),32))
   z.writestr('assets/minecraft/textures/items/leather_helmet.png',png((50,60,200,255)))
   z.writestr('assets/minecraft/textures/entity/test.png',png((1,2,3,255)))
  report=c.convert(pack,p/'out',base)
  assert 'terrain:stone' in report['converted'] and 'items:helmetCloth' in report['converted']
  assert report['animated_first_frame'] and report['unconverted_assets']
  with Image.open(p/'out/res/terrain.png') as im:
   assert im.getpixel((16,0))==(210,30,40,255) and im.getpixel((32,0))==(17,18,19,255)
  with Image.open(p/'out/res/items.png') as im:assert im.getpixel((0,0))==(50,60,200,255)
  try:c.convert(pack,p/'out',base)
  except ValueError:pass
  else:raise AssertionError('Overwrite allowed')
  for name,metadata in [('unsafe.zip',3),('wrong-version.zip',4)]:
   with zipfile.ZipFile(p/name,'w') as z:
    z.writestr('pack.mcmeta',json.dumps({'pack':{'pack_format':metadata}}))
    if name=='unsafe.zip':z.writestr('../escape.png',png((0,0,0,0)))
   try:c.convert(p/name,p/(name+'-out'),base)
   except ValueError:pass
   else:raise AssertionError('Invalid archive accepted')
   assert not (p/(name+'-out')).exists()
 print('PASS: atlas placement, legacy aliases, fallback preservation, animation report, version/path checks, no overwrite')
if __name__=='__main__':main()
