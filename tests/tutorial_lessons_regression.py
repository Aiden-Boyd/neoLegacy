"""Native area hints/profile opt-in and save-activation preservation regressions."""
from pathlib import Path
import importlib.util,struct,tempfile,zlib
from base_stability_regression import compile_and_run
from shulker_box_regression import method
ROOT=Path(__file__).resolve().parents[1]
FIXTURE=r'''
#pragma once
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
using DWORD=unsigned;using eINSTANCEOF=int;
struct Tile;struct ItemInstance;struct Entity;
enum eTutorial_State{e_Tutorial_State_Any=-2,e_Tutorial_State_None=-1,e_Tutorial_State_Gameplay=0,e_Tutorial_State_Test=1};enum eTutorial_Hint{e_Tutorial_Hint_Always_On=2};
struct Vec3{double x,y,z;};
struct AABB{double x0,y0,z0,x1,y1,z1;inline static int alive=0;~AABB(){--alive;}static AABB*newPermanent(double a,double b,double c,double d,double e,double f){++alive;return new AABB{a,b,c,d,e,f};}bool contains(Vec3*p){return p->x>x0&&p->x<x1&&p->y>y0&&p->y<y1&&p->z>z0&&p->z<z1;}};
struct LocalPlayer{int dimension=0;Vec3 pos{1,1,1};Vec3*getPos(int){return &pos;}};
struct Minecraft{shared_ptr<LocalPlayer>localplayers[4];inline static Minecraft*instance;static Minecraft*GetInstance(){return instance;}};
struct Tutorial{int pad=0;int messages=0;bool completed=false;eTutorial_State state=e_Tutorial_State_Gameplay;int getPad(){return pad;}eTutorial_State getCurrentState(){return state;}bool isStateCompleted(eTutorial_State){return completed;}void addMessage(int,bool){++messages;}};
'''
NATIVE_TEST=r'''
int main(){
 Minecraft mc;Minecraft::instance=&mc;Tutorial t;t.pad=1;mc.localplayers[0]=make_shared<LocalPlayer>();mc.localplayers[0]->pos={100,100,100};mc.localplayers[1]=make_shared<LocalPlayer>();
 auto hint=[&](eTutorial_State complete=e_Tutorial_State_None){return new AreaHint(e_Tutorial_Hint_Always_On,&t,e_Tutorial_State_Gameplay,complete,123,0,0,0,2,2,2);};
 {unique_ptr<TutorialHint>h(hint());assert(AABB::alive==1&&h->tick()==123&&h->tick()==-1);}assert(AABB::alive==0);
 for(int dimension:{-1,1}){unique_ptr<TutorialHint>h(hint());mc.localplayers[1]->dimension=dimension;assert(h->tick()==-1);mc.localplayers[1]->dimension=0;assert(h->tick()==123);}
 {unique_ptr<TutorialHint>h(hint());mc.localplayers[1].reset();assert(h->tick()==-1);mc.localplayers[1]=make_shared<LocalPlayer>();assert(h->tick()==123);}
 {unique_ptr<TutorialHint>h(hint());t.state=e_Tutorial_State_Test;assert(h->tick()==-1);t.state=e_Tutorial_State_Gameplay;mc.localplayers[1]->pos={0,1,1};assert(h->tick()==-1);mc.localplayers[1]->pos={1,1,1};assert(h->tick()==123);}
 {unique_ptr<TutorialHint>h(hint(e_Tutorial_State_Test));t.completed=true;assert(h->tick()==-1);}
 assert(AABB::alive==0);LevelGenerationOptions ordinary{0},other{46},marked{0x54553436};assert(!ordinary.isImportedTutorial()&&!other.isImportedTutorial()&&marked.isImportedTutorial());
}
'''

def native():
 base=ROOT/'Minecraft.Client/Common/Tutorial'
 with tempfile.TemporaryDirectory()as d:
  p=Path(d);(p/'stdafx.h').write_text(FIXTURE)
  for name in ['TutorialEnum.h','Tutorial.h']:(p/name).write_text('#include "stdafx.h"\n')
  (p/'TutorialHint.h').write_bytes((base/'TutorialHint.h').read_bytes());(p/'AreaHint.h').write_bytes((base/'AreaHint.h').read_bytes())
  cpp=(base/'AreaHint.cpp').read_text();cpp='\n'.join(l for l in cpp.splitlines()if not l.startswith('#include'))
  marker=method((ROOT/'Minecraft.Client/Common/GameRules/LevelGenerationOptions.h').read_text(),'bool isImportedTutorial()')
  code='#include "stdafx.h"\n#include "AreaHint.h"\nstruct LevelGenerationOptions{int m_4JDataValue;'+marker+'};\n'
  code+='TutorialHint::TutorialHint(eTutorial_Hint id,Tutorial*t,int description,eHintType type,bool fade):m_id(id),m_tutorial(t),m_descriptionId(description),m_type(type),m_hintNeeded(true),m_allowFade(fade){}\n'
  # Unused native hint methods still participate in the vtable; define fixture no-ops.
  code+='int TutorialHint::startDestroyBlock(shared_ptr<ItemInstance>,Tile*){return -1;}int TutorialHint::destroyBlock(Tile*){return -1;}int TutorialHint::attack(shared_ptr<ItemInstance>,shared_ptr<Entity>){return -1;}int TutorialHint::createItemSelected(shared_ptr<ItemInstance>,bool){return -1;}int TutorialHint::itemDamaged(shared_ptr<ItemInstance>){return -1;}bool TutorialHint::onTake(shared_ptr<ItemInstance>){return false;}bool TutorialHint::onLookAt(int,int){return false;}bool TutorialHint::onLookAtEntity(eINSTANCEOF){return false;}int TutorialHint::tick(){return -1;}\n'
  compile_and_run(p,'native_tutorial_area_hints',code+cpp+NATIVE_TEST)

def activation():
 spec=importlib.util.spec_from_file_location('activation',ROOT/'tools/activate_imported_tutorial.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 def i(n):return struct.pack('>I',n)
 def utf(v):b=v.encode();return struct.pack('>H',len(b))+b
 names=['MapOptions','dataTag','NamedArea','name','x0','y0','z0','x1','y1','z1'];lookup={n:i(k)for k,n in enumerate(names)}
 def node(name,attrs,children=()):return lookup[name]+i(len(attrs))+b''.join(lookup[k]+utf(v)for k,v in attrs.items())+i(len(children))+b''.join(children)
 areas=[node('NamedArea',dict(name=n,x0='0',y0='0',z0='0',x1='2',y1='2',z1='2'))for n in ['tutorialArea','bannerArea','slimeArea','elytraArea']]
 inner=struct.pack('>HB8s',2,0,bytes(8))+i(len(names))+b''.join(utf(n)for n in names)+i(0)+i(1)+node('MapOptions',{'dataTag':'0'},areas)+bytes(17)
 table=b'example localization bytes\xff';raw=i(len(table))+table+inner;marked,got=m.mark_rules(raw);assert raw[:4+len(table)]==marked[:4+len(table)]and len(got)==4
 rules=struct.pack('>h8sBII',1,bytes(8),1,len(m.rle_encode(raw)),len(raw))+m.rle_encode(raw)
 entries=[('level.dat',7,b'unchanged metadata'),('r.0.0.mcr',8,b'unchanged chunk bytes\xff'),('requiredGameRules.grf',9,rules)]
 body=b'';directory=b'';cursor=12
 for name,stamp,value in entries:body+=value;directory+=name.encode('utf-16-le').ljust(128,b'\0')+struct.pack('<IIq',len(value),cursor,stamp);cursor+=len(value)
 save=struct.pack('<IIhh',cursor,len(entries),7,9)+body+directory;save=struct.pack('<II',0,len(save))+zlib.compress(save)
 output,report=m.activate(save);decoded=m.read_save(output);assert decoded[:2]==entries[:2]and report['unchanged_virtual_files']==['level.dat','r.0.0.mcr'];assert m.unpack_rules(decoded[2][2])==marked
 output2,_=m.activate(output);assert output==output2
 for invalid in [b'',b'\0'*30]:
  try:m.read_save(invalid)
  except (ValueError,struct.error,zlib.error):pass
  else:raise AssertionError('Accepted malformed save')
 print('Explicit marker, deterministic activation, rule/region preservation and invalid input: passed')

def main():
 native();activation()
 s=(ROOT/'Minecraft.Client/MinecraftServer.cpp').read_text();a=s.index('app.m_gameRules.loadGameRules(ba_gameRules');b=s.index('csf->closeHandle(fe);',a);assert 'isImportedTutorial()'in s[a:b]and'app.SetTutorialMode(true)'in s[a:b]
 s=(ROOT/'Minecraft.Client/Common/Tutorial/FullTutorial.cpp').read_text();assert 'importedOptions->isImportedTutorial()'in s
 for area in ['bannerArea','slimeArea','elytraArea']:assert f'L"{area}"'in s
 assert 'content.length - bStringTable.length - sizeof(int)'in(ROOT/'Minecraft.Client/Common/GameRules/GameRuleManager.cpp').read_text()
 print('Save loading activates the native mode only for the marked import; newer hints use named areas: passed')
if __name__=='__main__':main()
