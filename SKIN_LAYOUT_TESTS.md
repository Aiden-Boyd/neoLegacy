# Skin texture/layout candidate

The skin-height lookup previously uploaded a fallback skin into the memory-skin
cache under the caller's URL. Default skins share an empty URL, so inspecting
one preview could make other previews and players bind its image. A temporarily
missing custom skin could also become stuck behind its fallback image.

The lookup now reads metadata without uploading or replacing memory skins.
Fallback heights are cached by texture enum, released images are deleted, and
texture-pack reload clears the metadata cache. Real custom images take priority
when available, including before their first GPU upload.

Preview arm width previously depended on GPU texture allocation IDs minus 37.
It now uses the default skin texture identity. World, equipment and first-person
passes use Player::getTexture rather than a magic numeric enum offset. Default
Alex variants use slim arms; DLC/custom skins retain the classic model because
there is no supported custom slim-arm metadata in this path.

Automated tests compile the production height lookup, memory-skin binding and
layout helper. They cover all eighteen defaults, shared URLs, mixed 32/64 pixel
heights, arbitrary GPU handles, pending/arriving custom skins, invalid fallback
IDs and cache invalidation. Full client compilation and existing regressions
remain required. GPU output and actual skin switching need in-game testing.

Manual checks:

1. Restart the game and cycle every default skin in both directions. Check the
   current preview and adjacent previews, including Steve/Alex variants.
2. Select several skins and check third person, first-person arms and inventory.
   Compare the left/right arm and leg textures and outer clothing layers.
3. Switch to a DLC skin, then back to defaults. Check unique geometry, armor,
   equipped heads, crouching and Elytra flight.
4. Reload texture packs and repeat. With a second player, select different
   default skins and check neither inherits the other's texture.
5. If a skin remains scrambled, capture its name and a screenshot, plus whether
   it happens in the selector, gameplay or both.
