from pathlib import Path
import tempfile
from base_stability_regression import compile_and_run
from shulker_box_regression import method
ROOT=Path(__file__).resolve().parents[1]
SUPPORT=r'''
#include <cassert>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;
struct Tag{enum{TAG_Byte=1,TAG_Short=2,TAG_Int=3,TAG_Long=4,TAG_Float=5,TAG_Double=6,TAG_String=8};virtual~Tag()=default;virtual int getId()=0;};
#define NUMBER(NAME,TYPE,KIND) struct NAME:Tag{TYPE data;explicit NAME(TYPE n):data(n){}int getId()override{return KIND;}};
NUMBER(ByteTag,signed char,1) NUMBER(ShortTag,short,2) NUMBER(IntTag,int,3) NUMBER(LongTag,long long,4) NUMBER(FloatTag,float,5) NUMBER(DoubleTag,double,6)
struct CompoundTag:Tag{int getId()override{return 10;}map<wstring,Tag*>tags;map<wstring,wstring>strings;map<wstring,CompoundTag*>children;bool contains(wstring k){return tags.count(k)||strings.count(k)||children.count(k);}bool contains(wstring k,int t){return t==8?strings.count(k):tags.count(k)&&tags[k]->getId()==t;}int getShort(wstring k){return tags.count(k)?static_cast<ShortTag*>(tags[k])->data:0;}int getInt(wstring k){return tags.count(k)?static_cast<IntTag*>(tags[k])->data:0;}int getByte(wstring k){return tags.count(k)?static_cast<ByteTag*>(tags[k])->data:0;}wstring getString(wstring k){return strings[k];}CompoundTag*getCompound(wstring k){return children[k];}Tag*copy(){return nullptr;}double getDouble(const wstring&);float getFloat(const wstring&);};
struct Item{struct Table{int length=4096;vector<Item*>values=vector<Item*>(4096);Item*&operator[](int n){return values.at(n);}};static Table items;};
inline Item::Table Item::items;
struct ItemInstance{int id=0,count=0,auxValue=0,popTime=0;CompoundTag*tag=nullptr;void load(CompoundTag*);static shared_ptr<ItemInstance>fromTag(CompoundTag*);Item*getItem(){return Item::items[id];}};
'''
TEST=r'''
int main(){
 Item item;Item::items[443]=&item;Item::items[2260]=&item;Item::items[4]=&item;
 CompoundTag tag;tag.strings[L"id"]=L"minecraft:elytra";ByteTag count(1);ShortTag damage(37);tag.tags[L"Count"]=&count;tag.tags[L"Damage"]=&damage;auto p=ItemInstance::fromTag(&tag);assert(p&&p->id==443&&p->count==1&&p->auxValue==37);
 tag.strings[L"id"]=L"minecraft:record_far";p=ItemInstance::fromTag(&tag);assert(p&&p->id==2260);tag.strings[L"id"]=L"4";p=ItemInstance::fromTag(&tag);assert(p&&p->id==4);
 tag.strings[L"id"]=L"mod:unknown";assert(!ItemInstance::fromTag(&tag));tag.strings.clear();IntTag overflow(999999);tag.tags[L"id"]=&overflow;assert(!ItemInstance::fromTag(&tag));ShortTag old(443);tag.tags[L"id"]=&old;p=ItemInstance::fromTag(&tag);assert(p&&p->id==443);
 CompoundTag values;FloatTag speed(.7f);DoubleTag precise(.125);LongTag wide(123456789);values.tags[L"speed"]=&speed;values.tags[L"precise"]=&precise;values.tags[L"wide"]=&wide;assert(abs(values.getDouble(L"speed")-.7)<.000001&&values.getFloat(L"precise")==.125f&&values.getDouble(L"wide")==123456789&&values.getDouble(L"missing")==0);
}
'''
def main():
 s=SUPPORT+'\n'+(ROOT/'Minecraft.World/LegacyItemNames.h').read_text()
 for signature in ['void ItemInstance::load(','shared_ptr<ItemInstance> ItemInstance::fromTag(']:s+='\n'+method((ROOT/'Minecraft.World/ItemInstance.cpp').read_text(),signature)
 source=(ROOT/'Minecraft.World/CompoundTag.h').read_text()
 for signature in ['double getDouble(','float getFloat(']:
  body=method(source,signature).replace(signature,signature.replace('get','CompoundTag::get',1),1);s+='\n'+body
 with tempfile.TemporaryDirectory()as d:compile_and_run(Path(d),'imported_nbt_types',s+TEST)
 print('Native namespaced item loading, old numeric IDs, rejected unknown IDs and numeric attribute coercion: passed')
if __name__=='__main__':main()
