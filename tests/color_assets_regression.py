from pathlib import Path
import hashlib,json
from content_assets_regression import png
ROOT=Path(__file__).resolve().parents[1];B=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res'
def main():
 m=json.loads((ROOT/'tests/color_atlas_manifest.json').read_text());assert len(m['cells'])==104 and len({tuple(c)for c in m['cells'].values()})==104
 for name,record in m['originals'].items():assert hashlib.sha256((B/'textures/blocks'/f'{name}.png').read_bytes()).hexdigest()==record['sha256'],name
 for filename,record in m['atlases'].items():
  w,h,p=png(B/filename);s=w//16;masked=bytearray(p)
  for row,col,digest in record['cells']:
   tile=b''.join(p[(y*w+col*s)*4:(y*w+(col+1)*s)*4]for y in range(row*s,(row+1)*s));assert hashlib.sha256(tile).hexdigest()==digest
   for y in range(row*s,(row+1)*s):masked[(y*w+col*s)*4:(y*w+(col+1)*s)*4]=bytes(s*4)
  assert hashlib.sha256(masked).hexdigest()==record['outside_sha256'],filename
 for color in ['white','orange','magenta','light_blue','yellow','lime','pink','gray','silver','cyan','purple','blue','brown','green','red','black']:
  _,_,base=png(B/'textures/blocks'/f'glazed_terracotta_{color}_r0.png')
  for q in range(1,4):
   w,h,actual=png(B/'textures/blocks'/f'glazed_terracotta_{color}_r{q}.png');expected=bytearray(w*h*4)
   for y in range(h):
    for x in range(w):
     xx,yy=x,y
     for _ in range(q):xx,yy=h-1-yy,xx
     expected[(yy*w+xx)*4:(yy*w+xx+1)*4]=base[(y*w+x)*4:(y*w+x+1)*4]
   assert actual==expected
 source=(ROOT/'Minecraft.Client/PreStitchedTextureMap.cpp').read_text()
 for name,(row,col)in m['cells'].items():assert f'ADD_ICON({row}, {col}, L"{name}")'in source
 print('104 atlas cells, original texture hashes, lossless glazed rotations, all mips and previous-pixel preservation: passed')
if __name__=='__main__':main()
