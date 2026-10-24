#pragma once
#include <string>
namespace LegacyEntityNames {
inline std::wstring entity(const std::wstring&name){
 struct Pair{const wchar_t*modern,*legacy;};static const Pair names[]={
 {L"minecraft:ender_crystal",L"EnderCrystal"},{L"minecraft:ender_dragon",L"EnderDragon"},{L"minecraft:horse",L"EntityHorse"},{L"minecraft:item_frame",L"ItemFrame"},{L"minecraft:painting",L"Painting"},{L"minecraft:armor_stand",L"ArmorStand"},{L"minecraft:cow",L"Cow"},{L"minecraft:pig",L"Pig"},{L"minecraft:chicken",L"Chicken"},{L"minecraft:sheep",L"Sheep"}};
 for(auto p:names)if(name==p.modern)return p.legacy;return name;
}
inline std::wstring tile(const std::wstring&name){
 struct Pair{const wchar_t*modern,*legacy;};static const Pair names[]={
 {L"minecraft:chest",L"Chest"},{L"minecraft:dispenser",L"Trap"},{L"minecraft:skull",L"Skull"},{L"minecraft:enchanting_table",L"EnchantTable"},{L"minecraft:furnace",L"Furnace"},{L"minecraft:hopper",L"Hopper"},{L"minecraft:comparator",L"Comparator"},{L"minecraft:brewing_stand",L"Cauldron"},{L"minecraft:dropper",L"Dropper"},{L"minecraft:noteblock",L"Music"},{L"minecraft:jukebox",L"RecordPlayer"},{L"minecraft:sign",L"Sign"},{L"minecraft:banner",L"Banner"}};
 for(auto p:names)if(name==p.modern)return p.legacy;return name;
}
}
