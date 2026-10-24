#pragma once
#include <string>
#include <vector>
#include <utility>
class CompoundTag;
struct BannerData {
    int base=2;
    bool cityStyle=true;
    std::wstring name;
    std::vector<std::pair<std::wstring,int> > patterns{{L"bt",0},{L"tt",0}};
    void plain(int color=15) {base=color&15;cityStyle=false;patterns.clear();name.clear();}
    void load(CompoundTag*, int itemBase=-1);
    void save(CompoundTag*) const;
    static const wchar_t *texture(const std::wstring &id);
    static int color(int dye);
};
