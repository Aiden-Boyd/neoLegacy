from pathlib import Path
import hashlib,json,struct,zlib
ROOT=Path(__file__).resolve().parents[1]
def png(path):
 b=path.read_bytes();assert b[:8]==b'\x89PNG\r\n\x1a\n';offset=8;data=b''
 while offset<len(b):
  size=struct.unpack('>I',b[offset:offset+4])[0];kind=b[offset+4:offset+8];chunk=b[offset+8:offset+8+size];offset+=12+size
  if kind==b'IHDR':width,height,depth,color,compression,flt,interlace=struct.unpack('>IIBBBBB',chunk);assert(depth,color,interlace)==(8,6,0)
  if kind==b'IDAT':data+=chunk
 raw=zlib.decompress(data);stride=width*4;out=bytearray();prev=bytearray(stride);offset=0
 def paeth(a,b,c):
  p=a+b-c;dist=[abs(p-a),abs(p-b),abs(p-c)];return[a,b,c][dist.index(min(dist))]
 for y in range(height):
  mode=raw[offset];row=bytearray(raw[offset+1:offset+1+stride]);offset+=stride+1
  for x in range(stride):
   left=row[x-4]if x>=4 else 0;up=prev[x];corner=prev[x-4]if x>=4 else 0
   pred=[0,left,up,(left+up)//2,paeth(left,up,corner)][mode];row[x]=(row[x]+pred)%256
  out+=row;prev=row
 return width,height,out

def verify_assets():
 base=ROOT/'Minecraft.Client/Common/res/TitleUpdate/res'
 records=json.loads((ROOT/'tests/content_atlas_hashes.json').read_text())
 for filename,record in records.items():
  width,height,pixels=png(base/filename);size=width//16;masked=bytearray(pixels)
  for row,col,expected in record['cells']:
   crop=b''.join(pixels[(y*width+col*size)*4:(y*width+(col+1)*size)*4]for y in range(row*size,(row+1)*size))
   assert hashlib.sha256(crop).hexdigest()==expected
   for y in range(row*size,(row+1)*size):masked[(y*width+col*size)*4:(y*width+(col+1)*size)*4]=bytes(size*4)
  assert hashlib.sha256(masked).hexdigest()==record['outside_sha256'],filename
 width,height,pixels=png(base/'textures/blocks/magma.png');assert(width,height)==(16,48)
 assert pixels[:width*width*4]!=pixels[width*width*4:width*width*8]
 assert (base/'textures/blocks/magma.txt').read_text().strip()=='0*8,1*8,2*8'
 assert (ROOT/'Minecraft.Client/Common/res/textures/blocks/magma.png').read_bytes()==(base/'textures/blocks/magma.png').read_bytes()
 source=(ROOT/'Minecraft.Client/PreStitchedTextureMap.cpp').read_text()
 assert 'textureFileName==L"magma"'in source and '0*8,1*8,2*8'in source
 for i,name in enumerate(['magma','nether_wart_block','red_nether_brick','bone_block_side','bone_block_top']+['beetroots_stage_'+str(i)for i in range(4)]):assert f'ADD_ICON(26, {i}, L"{name}")'in source
 print('New block/item cells, magma animation frames and preservation of all other atlas pixels: passed')
