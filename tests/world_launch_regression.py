"""Exercise the production pack-license lookup used before world loading.

The engine/UI are stubbed; the base getter and both launch lookup statements
are extracted from production. Full Create/Load gameplay still needs testing.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def main():
    header = (ROOT / 'Minecraft.Client/TexturePack.h').read_text()
    default_getter = re.search(r'virtual DLCPack \*getDLCInfoParentPack\(\)\s*\{[^}]*\}', header)
    assert default_getter, 'Base texture packs need a safe DLC ownership query'
    statements = []
    for scene in ('CreateWorld', 'Load'):
        source = (ROOT / f'Minecraft.Client/Common/UI/UIScene_{scene}Menu.cpp').read_text()
        start = source.index('// Check that we have the rights to use a texture pack')
        end = source.index('// do we have a license?', start)
        lookup = source[start:end]
        assert 'static_cast<DLCTexturePack' not in lookup
        match = re.search(r'm_pDLCPack\s*=.*?;', lookup)
        assert match
        statements.append(match.group())
    source = r'''
#include <cassert>
struct DLCPack { bool purchased; };
struct TexturePack {
    virtual ~TexturePack() = default;
    BASE_GETTER
};
struct LocalPack : TexturePack { unsigned long long canary=0xabcdef1234567890ULL; };
struct DLCPackTexture : TexturePack {
    DLCPack *owner;
    explicit DLCPackTexture(DLCPack *p):owner(p){}
    DLCPack *getDLCInfoParentPack() override { return owner; }
};
struct Launch {
    DLCPack *m_pDLCPack=nullptr;
    void create(TexturePack *pTexturePack) { CREATE_LOOKUP }
    void load(TexturePack *pTexturePack) { LOAD_LOOKUP }
};
int main() {
    TexturePack vanilla;
    LocalPack local;
    DLCPack purchased{true},trial{false};
    DLCPackTexture full(&purchased), demo(&trial), noOwner(nullptr);
    for(auto launch : { &Launch::create, &Launch::load }) {
        Launch menu;
        (menu.*launch)(&full); assert(menu.m_pDLCPack==&purchased);
        (menu.*launch)(&vanilla); assert(menu.m_pDLCPack==nullptr);
        (menu.*launch)(&demo); assert(menu.m_pDLCPack==&trial);
        (menu.*launch)(&local); assert(menu.m_pDLCPack==nullptr);
        assert(local.canary==0xabcdef1234567890ULL);
        (menu.*launch)(&full);
        (menu.*launch)(nullptr); assert(menu.m_pDLCPack==nullptr);
        (menu.*launch)(&noOwner); assert(menu.m_pDLCPack==nullptr);
    }
}
'''.replace('BASE_GETTER', default_getter.group()).replace('CREATE_LOOKUP',statements[0]).replace('LOAD_LOOKUP',statements[1])
    source = '#include <initializer_list>\n' + source
    compiler = os.environ.get('CXX') or shutil.which('g++') or shutil.which('clang++') or shutil.which('cl')
    assert compiler, 'A C++ compiler is required'
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        cpp = path / 'world_launch.cpp'
        exe = path / ('world_launch.exe' if os.name == 'nt' else 'world_launch')
        cpp.write_text(source)
        if Path(compiler).name.lower() in ('cl','cl.exe'):
            command = [compiler, '/nologo', '/EHsc', '/std:c++17', '/UNDEBUG', str(cpp), '/Fe:' + str(exe)]
        else:
            command = [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(exe)]
        subprocess.run(command, check=True, cwd=path)
        subprocess.run([str(exe)], check=True)
    print('PASS: Create/Load license lookup: default, local, full/trial DLC, missing pack, stale owner')

if __name__ == '__main__':
    main()
