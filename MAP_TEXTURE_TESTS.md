# Map colors and added terrain texture fallback

Base: 80d5b7b629650de5f9ebd57a4113f293dde16718. Cumulative candidate.

## Audit and changes

The base already has texture packs, whole-file fallback and texture reloads.
The confirmed gap is that a supplied terrain atlas prevents whole-file fallback,
even when its cells for our added blocks are blank. The existing loader is kept.
Only 124 explicitly allocated added terrain cells get per-cell fallback when
all base-level alpha values are zero. A cell with any supplied nonzero alpha is
preserved in every mip. Other cells are never modified. Default packs skip this
step, and the default atlas is loaded only if there is a missing added cell.
Fallback textures are nearest-scaled to the selected atlas resolution. Existing
mips are repaired in the same cell; native generation handles absent whole mips.
The helper accepts power-of-two atlases with the native 16-column/32-row layout.
It does not attempt to reinterpret differently arranged atlases. Fully transparent
intentional textures in added cells are treated as missing; nonblank placeholders
cannot be distinguished from supplied artwork and remain unchanged. Individual
item/entity file fallback stays on the native path. This is not a new pack system.

MapItem previously counted block IDs and selected material colors after sampling,
losing metadata. It now counts map color IDs per sampled block. Concrete/powder,
glazed terracotta, shulker boxes, purpur, chorus, End bricks, beetroot, path, frost,
magma, wart, red Nether brick, bone and observer mappings are covered. Wool,
carpets and stained terracotta also benefit from metadata-aware sampling. Legacy
color IDs 0..13 retain their pack-defined colors. New palette IDs 14..51 use the
1.12.2 RGB values; color/shade still fits the existing map byte/save/packet format.
New colors use default palette RGB, not new DLC color-table fields. Existing map
pixels retain their IDs until resampled; existing material IDs are not renumbered.
Minimap initializes all 256 lookup entries, reads map bytes unsigned, and handles
the fourth/darkest shade. Undefined palette entries render safely as black.

## Verification

Tests compile production MapItem::update, Minimap::reloadColours, MapPalette and
the exact fallback helper. They cover all dye metadata, representative added
blocks, mixed-color majority sampling, all 256 LUT entries, normal/high-resolution
atlas repair, all existing mip levels, supplied/partially-transparent cell
preservation, outside-cell preservation and idempotent reload. Existing asset
hash/atlas tests remain required. Full Windows client compilation is also required.

Reference: WangTingZheng/mcp940 d0c030a4139ce7cf3f284b180f0d9ea87bdf8141,
MapColor RGB palette and block-color ordering. No new textures were generated.

## In-game acceptance

1. Build broad swatches of every concrete/powder, glazed and shulker color;
   explore with a map, then check held and item-frame maps. Test scaled maps
   over mixed colors, water/land shading and Nether maps. Reload the world.
2. Switch between default and older texture packs. Check new block icons,
   near/far terrain, mip transitions and pack reloads. Existing pack blocks
   should keep their artwork; supplied nonblank new cells should keep theirs.
3. Try a higher-resolution pack and compare distant terrain. Intentionally
   blank added cells now show default textures. Nonblank placeholder art is
   retained by design. Test item/entity textures separately on native fallback.
4. Recheck portals, shulker boxes, villagers, Creative Misc pages and Steam input.

GPU rendering, actual pack switching and real save/network round trips remain
pending in-game checks. This batch does not change End dragon recovery.
