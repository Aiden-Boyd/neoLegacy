"""Verify imported geometry and markers before any world generation consumes them."""
from pathlib import Path
import importlib.util
import json
import tempfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('templates', ROOT / 'tools/compile_end_city_templates.py')
templates = importlib.util.module_from_spec(spec)
spec.loader.exec_module(templates)


def rejects(operation):
    try:
        operation()
    except ValueError:
        return
    raise AssertionError('Malformed input was accepted')


def main():
    directory = ROOT / 'assets/endcity'
    compiled = templates.compile_templates(directory)
    assert len(compiled) == 21
    assert compiled == templates.compile_templates(directory)
    assert compiled == json.loads((directory / 'templates.json').read_text())
    originals = {}
    import gzip
    for path in sorted(directory.glob('*.nbt')):
        originals[path.stem] = templates.NbtReader(gzip.decompress(path.read_bytes())).root()
    cells = 0
    for room in compiled:
        original = originals[room['name']]
        assert room['palette'] == original['palette']
        expanded = [state for count, state in room['runs'] for _ in range(count)]
        sx, sy, sz = room['size']
        assert len(expanded) == sx * sy * sz
        for block in original['blocks']:
            x, y, z = block['pos']
            assert expanded[(y * sz + z) * sx + x] == block['state']
        assert room['specials'] == [{'pos': b['pos'], 'nbt': b['nbt']}
                                    for b in original['blocks'] if 'nbt' in b]
        assert room['entities'] == original['entities']
        cells += len(expanded)
    assert cells == 23150
    ship = next(t for t in compiled if t['name'] == 'ship')
    markers = [s['nbt'].get('metadata') for s in ship['specials']]
    assert markers.count('Elytra') == 1 and markers.count('Chest') == 2 and markers.count('Sentry') == 3
    assert any(s['nbt'].get('SkullType') == 5 for s in ship['specials'])
    stand = next(s['nbt'] for s in ship['specials'] if s['nbt'].get('id') == 'Cauldron')
    assert {i['Slot'] for i in stand['Items']} == {0, 2}
    assert all(i['tag']['Potion'] == 'minecraft:strong_healing' for i in stand['Items'])
    assert any(s['nbt'].get('Patterns') for t in compiled for s in t['specials'])
    rejects(lambda: templates.NbtReader(b'').root())
    rejects(lambda: templates.NbtReader(b'\x01\0\0\0').root())
    rejects(lambda: templates.NbtReader(b'\x0a\0\0\0trailing').root())
    rejects(lambda: templates.NbtReader(b'\xff\xff\xff\xff').payload(7))
    rejects(lambda: templates.NbtReader(b'\xff').payload(99))
    # Verify corruption is detected independently of the NBT parser accepting it.
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        source = directory / 'base_floor.nbt'
        (root / source.name).write_bytes(source.read_bytes())
        (root / 'manifest.json').write_text(json.dumps({'base_floor': {'blob_sha': '0' * 40}}))
        rejects(lambda: templates.compile_templates(root))
    print('end_city_templates: passed (21 templates, 23,150 cells; all tile data and markers retained)')


if __name__ == '__main__':
    main()
