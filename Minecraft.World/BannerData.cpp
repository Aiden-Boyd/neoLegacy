#include "stdafx.h"
#include "BannerData.h"
#include "CompoundTag.h"
#include "ListTag.h"
void BannerData::load(CompoundTag *tag,int itemBase) {
    if(!tag->contains(L"Base",99)&&itemBase<0) {*this=BannerData();return;}
    plain(tag->contains(L"Base",99)?tag->getInt(L"Base"):itemBase);cityStyle=tag->getBoolean(L"EndCityStyle");
    if(tag->contains(L"CustomName",Tag::TAG_String))name=tag->getString(L"CustomName");
    if(!tag->contains(L"Patterns",Tag::TAG_List))return;
    auto list=tag->getList(L"Patterns");
    for(int i=0;i<list->size();++i)if(list->get(i)->getId()==Tag::TAG_Compound) {
        auto entry=static_cast<CompoundTag*>(list->get(i));
        patterns.emplace_back(entry->getString(L"Pattern"),entry->getInt(L"Color")&15);
    }
}
void BannerData::save(CompoundTag *tag) const {
    tag->putInt(L"Base",base&15);tag->putBoolean(L"EndCityStyle",cityStyle);
    if(!name.empty())tag->putString(L"CustomName",name);
    auto list=new ListTag<CompoundTag>();
    for(auto &p:patterns){auto entry=new CompoundTag();entry->putString(L"Pattern",p.first);entry->putInt(L"Color",p.second&15);list->add(entry);}
    tag->put(L"Patterns",list);
}
int BannerData::color(int dye) {
    static const int colors[16]={0x1d1d21,0xb02e26,0x5e7c16,0x835432,0x3c44aa,0x8932b8,0x169c9c,0x9d9d97,0x474f52,0xf38baa,0x80c71f,0xfed83d,0x3ab3da,0xc74ebd,0xf9801d,0xf9fffe};
    return colors[dye&15];
}
const wchar_t *BannerData::texture(const std::wstring &id) {
    if(id==L"bl")return L"square_bottom_left";
    if(id==L"br")return L"square_bottom_right";
    if(id==L"tl")return L"square_top_left";
    if(id==L"tr")return L"square_top_right";
    if(id==L"bs")return L"stripe_bottom";
    if(id==L"ts")return L"stripe_top";
    if(id==L"ls")return L"stripe_left";
    if(id==L"rs")return L"stripe_right";
    if(id==L"cs")return L"stripe_center";
    if(id==L"ms")return L"stripe_middle";
    if(id==L"drs")return L"stripe_downright";
    if(id==L"dls")return L"stripe_downleft";
    if(id==L"ss")return L"small_stripes";
    if(id==L"cr")return L"cross";
    if(id==L"sc")return L"straight_cross";
    if(id==L"bt")return L"triangle_bottom";
    if(id==L"tt")return L"triangle_top";
    if(id==L"bts")return L"triangles_bottom";
    if(id==L"tts")return L"triangles_top";
    if(id==L"ld")return L"diagonal_left";
    if(id==L"rd")return L"diagonal_up_right";
    if(id==L"lud")return L"diagonal_up_left";
    if(id==L"rud")return L"diagonal_right";
    if(id==L"mc")return L"circle";
    if(id==L"mr")return L"rhombus";
    if(id==L"vh")return L"half_vertical";
    if(id==L"hh")return L"half_horizontal";
    if(id==L"vhr")return L"half_vertical_right";
    if(id==L"hhb")return L"half_horizontal_bottom";
    if(id==L"bo")return L"border";
    if(id==L"cbo")return L"curly_border";
    if(id==L"cre")return L"creeper";
    if(id==L"gra")return L"gradient";
    if(id==L"gru")return L"gradient_up";
    if(id==L"bri")return L"bricks";
    if(id==L"sku")return L"skull";
    if(id==L"flo")return L"flower";
    if(id==L"moj")return L"mojang";
    return nullptr;
}
