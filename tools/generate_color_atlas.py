"""Rebuild derived World of Color textures and their allocated atlas cells.

Requires Pillow. Original PNGs and tests/color_atlas_manifest.json are committed.
Only allocated cells are written. Existing feature snapshots deliberately remain
unchanged so a changed source texture requires an explicit review of those hashes.
"""
from pathlib import Path
import argparse,json
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res'
def main():
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args();m=json.loads((ROOT/'tests/color_atlas_manifest.json').read_text());raw=BASE/'textures/blocks'
 for name in m['cells']:
  if len(name)>3 and name[-3:-1]=='_r' and name[-1]in '0123':
   original,q=name.rsplit('_r',1);expected=Image.open(raw/(original+'.png')).convert('RGBA').rotate(-90*int(q));dest=raw/(name+'.png')
   if args.check:assert Image.open(dest).convert('RGBA').tobytes()==expected.tobytes(),name
   else:expected.save(dest)
 for filename in m['atlases']:
  im=Image.open(BASE/filename).convert('RGBA');size=im.width//16
  for name,(row,col)in m['cells'].items():
   expected=Image.open(raw/(name+'.png')).convert('RGBA').resize((size,size),Image.Resampling.NEAREST);box=(col*size,row*size,(col+1)*size,(row+1)*size)
   if args.check:assert im.crop(box).tobytes()==expected.tobytes(),name+' '+filename
   else:im.paste(expected,box)
  if not args.check:im.save(BASE/filename)
 print('Derived rotations and allocated atlas cells match original assets')
if __name__=='__main__':main()
