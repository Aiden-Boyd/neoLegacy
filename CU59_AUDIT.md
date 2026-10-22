# Xbox One CU59 source audit

Audit date: 2026-10-22  
Repository: Aiden-Boyd/neoLegacy  
Audited branch: steam-proton-base  
Pinned commit: eaf2da084859e4990a3935c76527d9b07ac9a91e  
Successful client build: https://github.com/Aiden-Boyd/neoLegacy/actions/runs/39852461366

## Decision

Keep this exact build as the working Steam/Proton baseline. It is a TU19-derived
engine with substantial later additions, not a complete TU31 implementation and
not a CU59 implementation. A single update number or README percentage would
misrepresent what is present.

The owner reports successful play through Steam with controller input on Bazzite
and accepts this as the base. The Windows client compilation and runtime staging
also passed in GitHub Actions. Those establish a usable starting point, not
feature-by-feature parity, save migration compatibility, or multiplayer certification.

No game code was changed during this audit. The report is on a separate branch.

## Method and confidence

Inspected the complete Minecraft.World tree (1,948 entries; not truncated), the
block/item/entity/tile-entity/enchantment/effect registries, build source lists,
selected recipes, world generation, movement, vehicles, bosses, and relevant
client/UI paths. Inspected the Common/UI tree (277 entries) and GameRules and
Tutorial trees. Reads were pinned to the commit above.

- **Implemented paths:** actual code plus registration/integration found; not a
  claim of faithful behavior until tested.
- **Partial:** meaningful code exists, but related required systems are missing
  or a concrete difference was found.
- **Missing in inspected implementation:** absence corroborated by registries,
  build paths or related systems, rather than only a filename search.
- **Unverified:** requires runtime comparison, binary asset inspection, or further
  targeted source work.

This is a source-level gap audit, not an exhaustive comparison of every recipe,
texture, bug fix, achievement, generated structure, seed, or network packet.
No gameplay tests were run by the auditor and no completion percentage is assigned.

## Reference target

The target remains Minecraft: Xbox One Edition CU59, with vanilla console
behavior and the owner's Steam/Proton requirements. It is not Java 1.12.2 parity.
Use Java as a reference for individual algorithms, then reconcile with console
behavior. Do not automatically import Java combat, shields, advancements, recipes,
or later updates.

The official 2016 console announcement establishes the End expansion, additional
mobs/items, brewing fuel and tutorial changes. The 2017 console announcement
establishes the exploration/color-era systems. The 4J Xbox One changelog establishes
later console-specific changes and fixes. Xbox One Legacy did not receive Aquatic.

Reference sources:
- https://www.minecraft.net/en-us/article/update-110-and-more-console-today
- https://www.minecraft.net/en-us/article/console-updates-continue
- https://www.minecraftforum.net/forums/minecraft-editions/minecraft-xbox-one-edition/mcxone-discussion/2230175-xbox-one-content-update-change-logs
- https://www.minecraft.net/nb-no/article/update-aquatic-coming-legacy-console-editions

## What is already worth preserving

| System | Finding | Source evidence |
| --- | --- | --- |
| Steam launch and controller input | Owner-verified baseline on Bazzite; deeper edge cases remain unverified | Successful build above; STEAM_PROTON.md |
| Windows renderer/input | Existing D3D11 and XInput integration; preserve during content work | [Minecraft.Client/CMakeLists.txt](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.Client/CMakeLists.txt) |
| Later biome infrastructure | Mesa, savanna, roofed forest and deep-ocean definitions exist; exact generation parity unverified | [Minecraft.World/Biome.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Biome.cpp) |
| Building variants | Stone variants, stained glass, packed ice, red sandstone and wood door/fence variants have real registrations | [Minecraft.World/Tile.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Tile.cpp); [Minecraft.World/StoneTile.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/StoneTile.cpp) |
| Rabbits | Entity registration, behavior, drops/breeding, renderer and creative entry exist | [Minecraft.World/Rabbit.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Rabbit.cpp); [Minecraft.World/EntityIO.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/EntityIO.cpp); [Minecraft.Client/EntityRenderDispatcher.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.Client/EntityRenderDispatcher.cpp) |
| Guardians and elder guardians | Registrations, AI/rendering and monument spawn code exist; all acquisition paths and elder behavior unverified | [Minecraft.World/Guardian.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Guardian.cpp); [Minecraft.World/OceanMonumentPieces.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/OceanMonumentPieces.cpp) |
| Monuments | Structure placement, postprocessing and structure-specific mob selection are integrated | [Minecraft.World/RandomLevelSource.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/RandomLevelSource.cpp); [Minecraft.World/OceanMonumentFeature.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/OceanMonumentFeature.cpp) |
| Armor stands | Entity, placement item, equipment/pose serialization, renderer, recipe and creative entry exist; late-console interaction differs | [Minecraft.World/ArmorStand.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/ArmorStand.cpp); [Minecraft.World/Recipes.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Recipes.cpp) |
| Enchanting | Lapis slot and consumption exist; Depth Strider-style WaterWalker enchantment is registered | [Minecraft.World/EnchantmentMenu.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/EnchantmentMenu.cpp); [Minecraft.World/Enchantment.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Enchantment.cpp) |
| Elytra | Registered item and creative entry; player activation, flight movement, durability and collision logic exist | [Minecraft.World/ElytraItem.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/ElytraItem.cpp); [Minecraft.World/Player.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Player.cpp); [Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.Client/Common/UI/IUIScene_CreativeMenu.cpp) |

Existing code should be tested and repaired rather than reimplemented wholesale.

## Main gaps to the target

| Area | Status | Evidence and implication |
| --- | --- | --- |
| Slime blocks and attached piston structures | Missing | Tile.h leaves ID 165 as a comment; Tile.cpp has no slime-block registration. PistonBaseTile.cpp still requires a separate review for linked-block movement. A slime mob and slimeballs do not provide slime blocks. |
| Banners | Missing | Block IDs 176/177 and item ID 425 are only comments; no banner entity/storage or crafting scene found in inspected paths. Requires recipes, patterns, rendering, save/network data and console UI. |
| End cities/ships, chorus, purpur and End gateways | Missing | Tile.h lists future End IDs as comments; no corresponding block registrations or structure classes. TheEndLevelRandomLevelSource.cpp is the older generator. |
| Complete End progression | Partial | DragonFireball and EndPodiumFeature exist, but they do not establish outer-island exploration, city/ship loot or a complete respawn ritual. Elytra cannot be counted as normal survival progression without its acquisition chain. |
| Shulkers | Missing | No entity registration or implementation; no shulker block entity. Levitation is also absent from the effect registry. |
| Lingering potions, tipped arrows and later effects | Missing | No corresponding item registrations; MobEffect.cpp leaves later effect slots null. Must integrate potion data, projectiles, effects, visuals and inventory behavior. |
| Blaze-powder brewing fuel | Missing | BrewingStandTileEntity uses ingredient slot 3 and the older brewing state; no fuel field or fuel slot. Client menu and synchronization need updating too. |
| Mending/Frost Walker and later enchantments | Missing | No registrations in Enchantment.cpp or corresponding implementation classes. Update application rules, UI, acquisition and persistence together. |
| Wood-type/two-seat paddle boats | Missing newer implementation | Boat.cpp uses the older single-rider path and empty extra save-data methods; Item.cpp has the old boat item. Do not assume later boats from the presence of Boat.cpp. |
| Firework Elytra boosting | Missing in inspected paths | FireworksItem.cpp and FireworksRocketEntity.cpp have no Elytra attachment/boost path. Existing rocket movement is independent. |
| Polar bears, husks and strays | Missing | No dedicated registrations; reviewed Skeleton.cpp and Zombie.cpp do not establish those later variants. Skeleton trap-horse behavior remains unverified. |
| Beetroots, grass paths, frosted ice and later Nether building blocks | Missing | Relevant block IDs are comments, not registrations. Need items/recipes, behavior and generation integration where applicable. |
| Woodland mansions and treasure-map progression | Missing | No mansion structure implementation or later illager registrations; map/trade/structure integration needs implementing and testing. |
| Llamas, parrots, vindicators, evokers and vexes | Missing | No registrations in EntityIO.cpp and no matching implementation classes in the complete world tree. |
| Totem of Undying | Missing | No item registration or death-interception implementation identified in inspected paths. |
| Concrete/powder and glazed terracotta | Missing | No block registrations or implementation classes. Requires all variants, powder conversion/falling, orientation, recipes and UI entries. |
| Colored/bouncy beds | Missing later behavior | Reviewed BedTile.cpp has the older implementation; no color or bounce logic. Block data, item variants, rendering and sleep/respawn safety all need review. |
| Observer redstone | Missing | No observer registration/implementation. Later observer/piston edge cases cannot be audited as supported. |
| Late-console cauldrons | Missing extensions | CauldronTile.cpp handles water, bottle filling and clearing leather color; it does not implement stored potions, dyed water or tipped-arrow production. |
| Leather horse armor/new horse presentation | Missing/partial | EntityHorse.cpp only lists iron/gold/diamond armor. Exact model/animation and horse behavior parity need further comparison. |
| Late-console armor stand poses | Partial | Pose NBT exists, but default arms are disabled and no player pose-cycling path was found in the reviewed interaction code. |
| Late-console Wither behavior | Older implementation | WitherBoss.cpp still regenerates health and does not show the complete later console phase behavior. Needs a console reference, not a Java-only replacement. |
| Later villages and fallen/dying trees | Missing/partial | VillagePieces.cpp retains pressure-plate table placements and no grass-path use. Exact generator changes require a separate seed/structure comparison. |
| Battle, Tumble and Glide | Missing functional implementation in inspected paths | Main-menu Mini Games handling routes to the ordinary game flow. No matching mode scenes or implementations found in the reviewed UI, GameRules and build lists. |
| Final tutorial world and lessons | Unverified/incomplete | Tutorial framework exists, but later lesson hooks and packaged world identity were not established. Binary map presence is not proof that this is the final tutorial. |
| Final bug fixes and full visual/audio parity | Unverified | Requires separate regression cases, asset inventory and reference captures. Existing files or a successful build do not establish exact parity. |

Principal gap evidence:
- [Minecraft.World/Tile.h](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Tile.h)
- [Minecraft.World/Tile.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Tile.cpp)
- [Minecraft.World/Item.h](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Item.h)
- [Minecraft.World/Item.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Item.cpp)
- [Minecraft.World/EntityIO.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/EntityIO.cpp)
- [Minecraft.World/TileEntity.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/TileEntity.cpp)
- [Minecraft.World/MobEffect.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/MobEffect.cpp)
- [Minecraft.World/Enchantment.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Enchantment.cpp)
- [Minecraft.World/Boat.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Boat.cpp)
- [Minecraft.World/BrewingStandTileEntity.h](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/BrewingStandTileEntity.h)
- [Minecraft.World/CauldronTile.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/CauldronTile.cpp)
- [Minecraft.World/TheEndLevelRandomLevelSource.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/TheEndLevelRandomLevelSource.cpp)
- [Minecraft.Client/Common/UI/UIScene_MainMenu.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.Client/Common/UI/UIScene_MainMenu.cpp)

## Concrete code findings to investigate first

### 1. Sponge wetness is stored on a shared block object

Confidence: high static-code finding; gameplay reproduction pending.

Tile.cpp constructs one Sponge object for the tile type. Sponge.h adds a mutable
wet member to that shared object. onPlace overwrites it from one block's metadata,
while neighborChanged/tryAbsorb and animateTick consult the shared member instead
of reading the particular block's metadata. One sponge can therefore influence
how another is treated.

Reproduction: place a wet sponge and a dry sponge in separate water areas; change
neighboring water and vary placement/update order. Repeat after chunk unload and
save/reload. Expected: each block's wetness is independent.

Evidence: [Minecraft.World/Sponge.h](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Sponge.h);
[Minecraft.World/Sponge.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Sponge.cpp); [Minecraft.World/Tile.cpp](https://github.com/Aiden-Boyd/neoLegacy/blob/eaf2da084859e4990a3935c76527d9b07ac9a91e/Minecraft.World/Tile.cpp).

### 2. The Mini Games menu entry is not a functional minigame selector

Confidence: high in the inspected Windows menu code.

UIScene_MainMenu.cpp's eControl_MiniGames case selects eAction_RunGame and the
same CreateLoad_SignInReturned callback as normal Play. An available-looking
button must not be counted as implemented Battle/Tumble/Glide.

Reproduction: select Mini Games on this build and document the resulting screen.
Keep actual modes as an explicit content milestone.

### 3. Monument reload reconstruction has an integration gap

Confidence: static integration concern, not a reproduced world bug.

RandomLevelSource.cpp includes oceanMonument in generation, postprocessing and
spawn selection, but recreateLogicStructuresForChunk only reapplies mineshafts,
villages, strongholds and scattered structures. It omits monuments.

This may affect recognition after loading existing chunks; do not assume it
definitely breaks all reloads, because structure prescanning/storage may cover
some cases. Test guardian spawning/structure recognition at known monuments
before and after restarts, including a monument well away from the initial area.

### 4. Elytra is present, but not a finished parity feature

Confidence: strong source evidence of a partial implementation.

Item registration, creative selection, player movement and durability exist.
However, survival acquisition and the later firework boost path are missing.
ElytraItem::use only plays a sound and returns the item; verify the intended
equipment interaction through the inventory rather than assuming use-to-equip.

Test activation on controller, equipping, low durability, landing/collision
damage, saving while equipped, other players seeing flight, and split-screen.
Do not label it complete based on a creative flight demonstration.

## Backport order

1. Preserve baseline commit and download/archive the successful runtime locally.
   Before new content, establish repeatable smoke tests for launch, controller
   menus, world create/save/reload, Nether/End travel and clean exit.
2. Repair inherited integration issues and finish earlier missing content:
   sponge independence, monument reload behavior, slime blocks/piston structures,
   and banners with complete console crafting/pattern UI.
3. Add the shared foundations for later updates: effect and potion types,
   brewing fuel, enchantments, vehicle/equipment/network representations, and
   save-ID migration rules. Preserve existing IDs and back up test worlds.
4. Complete End progression: generator, structures, blocks, mobs, loot,
   acquisition and dragon lifecycle. Validate Elytra and boosting together.
5. Add exploration/color-era content as complete vertical slices: structure/mob
   spawning, trades/maps, drops, recipes, inventory icons, saving and rendering.
6. Add remaining late-console mechanics and targeted regression fixes.
7. Treat minigames, final tutorial/assets and exact visual/audio matching as
   explicit workstreams before calling the result CU59 parity.

Every slice needs single-player behavior, save/reload, controller/UI access,
rendering/audio, and multiplayer synchronization checks where applicable.

## Scope exclusions and unanswered questions

- No renderer rewrite or native Linux migration is justified by this audit.
  The existing Windows/Proton path already satisfies the owner's immediate need.
- Xbox Live services, achievements and original console account integration are
  separate from faithful local gameplay; no claim of restoration is made.
- Packaged DLC completeness and licensing were not audited.
- Split-screen, controller reconnect/vibration, networking, long-session memory
  behavior, exact chunk generation and performance were not runtime-tested.
- No merge of athena-main is recommended solely by its name. It adds two
  animal-behavior commits; they require independent target-version review.
- CU59 is a target label, not the current build's version. Do not change the
  displayed version to imply content that has not been implemented and verified.
