"""Compile the original NBT rooms into immutable, engine-independent templates.

Run this when changing assets/endcity, then commit the generated JSON with them.
Air is retained: a room's interior matters as much as its walls. Tile-entity and
structure-marker payloads are retained separately instead of silently dropping loot.
"""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parents[1]


class NbtReader:
    def __init__(self, data):
        self.data = memoryview(data)
        self.offset = 0

    def take(self, length):
        if length < 0 or self.offset + length > len(self.data):
            raise ValueError('Truncated or negative-length NBT field')
        out = self.data[self.offset:self.offset + length]
        self.offset += length
        return out

    def number(self, fmt):
        return struct.unpack('>' + fmt, self.take(struct.calcsize('>' + fmt)))[0]

    def string(self):
        return self.take(self.number('H')).tobytes().decode('utf-8')

    def payload(self, kind, depth=0):
        if depth > 64:
            raise ValueError('NBT nesting exceeds 64 levels')
        if kind in (1, 2, 3, 4, 5, 6):
            return self.number({1: 'b', 2: 'h', 3: 'i', 4: 'q', 5: 'f', 6: 'd'}[kind])
        if kind == 8:
            return self.string()
        if kind == 10:
            result = {}
            while True:
                child = self.number('B')
                if child == 0:
                    return result
                name = self.string()
                if name in result:
                    raise ValueError('Duplicate NBT key: ' + name)
                result[name] = self.payload(child, depth + 1)
        if kind in (7, 9, 11, 12):
            child = self.number('B') if kind == 9 else {7: 1, 11: 3, 12: 4}[kind]
            count = self.number('i')
            if count < 0 or count > len(self.data):
                raise ValueError('Invalid NBT array length')
            return [self.payload(child, depth + 1) for _ in range(count)]
        raise ValueError('Unsupported NBT tag: ' + str(kind))

    def root(self):
        if self.number('B') != 10:
            raise ValueError('Structure root must be a compound')
        self.string()
        result = self.payload(10)
        if self.offset != len(self.data):
            raise ValueError('Trailing data after NBT root')
        return result


def read_template(path):
    raw = path.read_bytes()
    root = NbtReader(gzip.decompress(raw)).root()
    size = root['size']
    if len(size) != 3 or any(not isinstance(v, int) or not 0 < v <= 64 for v in size):
        raise ValueError('Invalid template dimensions: ' + path.name)
    palette = root['palette']
    cells = {}
    specials = []
    for block in root['blocks']:
        pos, state = block['pos'], block['state']
        if len(pos) != 3 or any(v < 0 or v >= size[i] for i, v in enumerate(pos)):
            raise ValueError('Block outside template: ' + path.name)
        if not 0 <= state < len(palette):
            raise ValueError('Invalid palette index: ' + path.name)
        index = (pos[1] * size[2] + pos[2]) * size[0] + pos[0]
        if index in cells:
            raise ValueError('Duplicate template position: ' + path.name)
        cells[index] = state
        if 'nbt' in block:
            specials.append({'pos': pos, 'nbt': block['nbt']})
    volume = size[0] * size[1] * size[2]
    if len(cells) != volume:
        raise ValueError('Sparse templates need explicit structure-void support')
    runs = []
    for index in range(volume):
        state = cells[index]
        if runs and runs[-1][1] == state:
            runs[-1][0] += 1
        else:
            runs.append([1, state])
    blob_sha = hashlib.sha1(b'blob ' + str(len(raw)).encode() + b'\0' + raw).hexdigest()
    return {'name': path.stem, 'size': size, 'palette': palette, 'runs': runs,
            'specials': specials, 'entities': root.get('entities', []), 'blob_sha': blob_sha}


def compile_templates(directory):
    manifest = json.loads((directory / 'manifest.json').read_text())
    result = []
    for name, provenance in sorted(manifest.items()):
        template = read_template(directory / (name + '.nbt'))
        if template['blob_sha'] != provenance['blob_sha']:
            raise ValueError('Asset hash differs from manifest: ' + name)
        result.append(template)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--assets', type=Path, default=ROOT / 'assets/endcity')
    parser.add_argument('--output', type=Path, default=ROOT / 'assets/endcity/templates.json')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    content = json.dumps(compile_templates(args.assets), sort_keys=True, separators=(',', ':')) + '\n'
    if args.check:
        if args.output.read_text() != content:
            raise SystemExit('Generated End city templates are out of date')
    else:
        args.output.write_text(content)


if __name__ == '__main__':
    main()
