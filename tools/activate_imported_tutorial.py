"""Opt a converted Win64 TU46 save into native tutorial lessons, preserving its regions.

Input must be a v7/v9 little-endian saveData.ms from the separate conversion.
For Xbox LZX rules, supply LCEStudio's MIT codec.py with --codec-dir.
No world or proprietary rule data is included in this tool.
"""
from pathlib import Path
import argparse,hashlib,importlib.util,json,struct,zlib
MAGIC=0x54553436

def rle_decode(data):
 out=bytearray();i=0
 while i<len(data):
  value=data[i];i+=1
  if value!=255:out.append(value);continue
  count=data[i];i+=1
  if count<=2:out+=bytes([255])*(count+1)
  else:out+=bytes([data[i]])*(count+1);i+=1
 return bytes(out)

def rle_encode(data):
 # Literal-only encoding is valid RLE and does not depend on a platform codec.
 return data.replace(b'\xff',b'\xff\x00')

def unpack_rules(data,codec_dir=None):
 version,padding,compression,length,size=struct.unpack_from('>h8sBII',data)
 if version!=1 or length>len(data)-19 or size>16*1024*1024:raise ValueError('Unsupported rule envelope')
 payload=data[19:19+length]
 if compression in (0,1):raw=rle_decode(payload)
 elif compression==2:
  if codec_dir is None:raise ValueError('Xbox rules require --codec-dir containing LCEStudio codec.py')
  spec=importlib.util.spec_from_file_location('lce_tutorial_codec',Path(codec_dir)/'codec.py');codec=importlib.util.module_from_spec(spec);spec.loader.exec_module(codec)
  raw,expected=codec._inflate_chunk(struct.pack('>II',length,size)+payload)
  if expected!=size:raise ValueError('Invalid LZX rule size')
 else:raise ValueError('Unsupported rule compression')
 if len(raw)!=size:raise ValueError('Decompressed rule size mismatch')
 return raw

def mark_rules(raw):
 # Keep the original string table, coordinates, task rules and padding bytes.
 table_size=struct.unpack_from('>I',raw)[0];start=4+table_size;p=start
 version=struct.unpack_from('>H',raw,p)[0];p+=2
 compression=raw[p];p+=9
 if version not in (1,2)or compression!=0:raise ValueError('Unsupported inner rule format')
 def integer():
  nonlocal p
  value=struct.unpack_from('>I',raw,p)[0];p+=4;return value
 def utf():
  nonlocal p
  n=struct.unpack_from('>H',raw,p)[0];p+=2
  value=raw[p:p+n].decode('utf8');p+=n;return value
 strings=[utf()for _ in range(integer())]
 for _ in range(integer()):utf();n=integer();p+=n
 patches=[];areas={}
 def node():
  name=strings[integer()];attrs={}
  for _ in range(integer()):
   attr=strings[integer()];a=p;value=utf();attrs[attr]=value
   if name=='MapOptions'and attr=='dataTag':patches.append((a,p,struct.pack('>H',len(str(MAGIC)))+str(MAGIC).encode()))
  if name=='NamedArea':areas[attrs['name']]=[int(attrs[c])for c in ['x0','y0','z0','x1','y1','z1']]
  for _ in range(integer()):node()
 for _ in range(integer()):node()
 if len(patches)!=1 or not {'tutorialArea','bannerArea','slimeArea','elytraArea'}<=areas.keys():raise ValueError('Not the supported TU46 tutorial rule set')
 a,b,value=patches[0];marked=raw[:a]+value+raw[b:]
 return marked,areas

def read_save(data):
 zero,size=struct.unpack_from('<II',data);raw=zlib.decompress(data[8:])
 if zero!=0 or len(raw)!=size:raise ValueError('Not a converted Win64 save')
 footer,count,original,current=struct.unpack_from('<IIhh',raw)
 if(original,current)!=(7,9)or footer+144*count!=len(raw):raise ValueError('Unsupported save layout')
 entries=[]
 for i in range(count):
  a=footer+144*i;name=raw[a:a+128].decode('utf-16-le').split('\0')[0];n,start,stamp=struct.unpack_from('<IIq',raw,a+128)
  if start<12 or start+n>footer:raise ValueError('Invalid VFS entry')
  entries.append((name,stamp,raw[start:start+n]))
 if len({e[0]for e in entries})!=len(entries):raise ValueError('Duplicate virtual files')
 return entries

def activate(data,codec_dir=None):
 entries=read_save(data);output=[];changed=0;areas={}
 for name,stamp,body in entries:
  if name=='requiredGameRules.grf':
   original=unpack_rules(body,codec_dir);marked,areas=mark_rules(original);packed=rle_encode(marked);body=struct.pack('>h8sBII',1,bytes(8),1,len(packed),len(marked))+packed
   assert unpack_rules(body)==marked;changed+=1
  output.append((name,stamp,body))
 if changed!=1:raise ValueError('Expected exactly one requiredGameRules.grf')
 body=bytearray();table=bytearray();cursor=12
 for name,stamp,value in output:
  body+=value;table+=name.encode('utf-16-le').ljust(128,b'\0')+struct.pack('<IIq',len(value),cursor,stamp);cursor+=len(value)
 raw=struct.pack('<IIhh',cursor,len(output),7,9)+body+table;result=struct.pack('<II',0,len(raw))+zlib.compress(raw,6)
 reread=read_save(result);unchanged=[]
 for old,new in zip(entries,reread):
  assert old[:2]==new[:2]
  if old[0]!='requiredGameRules.grf':
   assert old[2]==new[2];unchanged.append(old[0])
 return result,{'marker':MAGIC,'areas':areas,'unchanged_virtual_files':unchanged,'save_sha256':hashlib.sha256(result).hexdigest(),'in_game_tested':False}

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('output',type=Path);p.add_argument('--codec-dir',type=Path);a=p.parse_args()
 if a.output.exists():raise FileExistsError(a.output)
 result,report=activate(a.source.read_bytes(),a.codec_dir);a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_bytes(result);print(json.dumps(report,indent=2))
if __name__=='__main__':main()
