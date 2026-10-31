"""Build a deterministic source-only Horizon native bundle (no game files)."""
from pathlib import Path
import hashlib
import json
import tomllib
import zipfile

root = Path(__file__).resolve().parents[1]
manifest = json.loads((root / 'horizon-native.json').read_text())
folder = root / 'mixins' / manifest['id']
mixin = tomllib.loads((folder / 'horizon-mixin.toml').read_text())
assert manifest['schema'] == 1
assert mixin['mod']['id'] == manifest['id']
assert mixin['mod']['version'] == manifest['version']
assert mixin['compatibility']['horizon_revision'] == manifest['horizon']
for key in ('horizon', 'mixins'):
    assert len(manifest[key]) == 40 and all(c in '0123456789abcdef' for c in manifest[key])
for hook in mixin['injection']:
    assert (folder / hook['helper']).is_file()
for module in mixin['module']:
    assert (folder / module['source']).is_file()
files = [root / 'horizon-native.json'] + sorted(folder.iterdir())
assert len(files) <= 512 and all(p.is_file() and not p.is_symlink() for p in files)
output = root / 'dist' / f"lce-crafting-{manifest['version']}-horizon-26.3.hnmod"
output.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as bundle:
    for path in files:
        info = zipfile.ZipInfo(path.relative_to(root).as_posix(), (2026, 1, 1, 0, 0, 0))
        info.compress_type = zipfile.ZIP_DEFLATED
        info.external_attr = 0o100644 << 16
        bundle.writestr(info, path.read_bytes())
with zipfile.ZipFile(output) as bundle:
    assert bundle.testzip() is None
print(output)
print('SHA256', hashlib.sha256(output.read_bytes()).hexdigest())
