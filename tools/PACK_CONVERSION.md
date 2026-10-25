# Java 1.12.2 resource conversion

Requires Python 3 and Pillow (`python -m pip install Pillow`). From the repo root:

    python tools/convert_java_pack.py PACK.zip NEW_OUTPUT_DIRECTORY

Only pack_format 3 ZIPs with pack.mcmeta at their root are accepted. The source
is read without extraction or modification. Existing output directories are
rejected. Use --base-res PATH to choose another native base resource directory.
It must contain terrain.png and items.png with the engine's native atlas layout.
The engine's PreStitchedTextureMap.cpp is the authority for atlas slots.

This is an initial offline block/item atlas converter, NOT a full pack installer
or DLC container writer. Output res/terrain.png and res/items.png are native
resource overlays. They are not selectable in-game by copying this directory
into the game. Runtime folder-pack registration/container integration is pending.
No proprietary pack assets are included in this tool.

Missing mappings retain the supplied native base artwork. conversion-report.json
lists converted slots, retained fallback slots, first-frame animations and all
unconverted assets. Legacy camel-case item names and selected legacy block names
are mapped; unresolved names remain explicit fallbacks. Multi-cell ADD_ICON_SIZE
slots are not converted. Atlas resolution currently follows the base atlas and
higher-resolution input sprites are nearest-neighbor downsampled. Animations use
the first frame only; animation timing, entity layouts, GUI, sounds, Java custom
models and custom namespaces are not converted. Do not call an output a complete
or visually verified pack until those gaps are resolved for that specific input.

Validation:

    python tests/pack_conversion_regression.py

Synthetic artwork tests atlas placement, aliases, fallback preservation, animation
reporting, unsafe ZIP paths, format rejection and overwrite prevention. No real
pack conversion or in-game pack acceptance has been verified yet.
