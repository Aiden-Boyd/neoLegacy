"""Windows storage integration: compile production save/load methods and use real files."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Minecraft.Client/Windows64/4JLibs/impls/Windows_Libs/Storage/src/STO_SaveGame.cpp"

def method(source, signature):
    start = source.index(signature)
    end = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end] + "\n"

STUB = r'''
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
struct C4JStorage { enum ESaveGameState { ESaveGame_Idle }; };
struct SAVE_INFO { char UTF8SaveFilename[32]; char UTF8SaveTitle[256]; };
using PSAVE_INFO = SAVE_INFO*;
struct CSaveGame {
    void* m_pSaveData = nullptr;
    unsigned int m_uiSaveSize = 0;
    char m_szSaveUniqueName[32] = "test";
    char m_szSaveTitle[256] = "A:B/World";
    bool m_bIsSafeDisabled = false;
    PBYTE m_pbThumbnailData = nullptr;
    unsigned int m_uiThumbnailSize = 0;
    void ResetSaveData();
    void SetSaveUniqueFilename(char*);
    C4JStorage::ESaveGameState SaveSaveData(int (*)(LPVOID,const bool),LPVOID);
    C4JStorage::ESaveGameState LoadSaveData(PSAVE_INFO,int (*)(LPVOID,const bool,const bool),LPVOID);
};
static bool result = false;
int saved(LPVOID, const bool success) { result = success; return 0; }
int loaded(LPVOID, const bool, const bool success) { result = success; return 0; }
'''
TEST = r'''
int main() {
    CSaveGame save;
    save.m_pSaveData = malloc(4);
    save.m_uiSaveSize = 4;
    memcpy(save.m_pSaveData,"good",4);
    result = false;
    save.SaveSaveData(saved,nullptr);
    assert(result);
    SAVE_INFO info = {};
    strcpy_s(info.UTF8SaveFilename,"test");
    strcpy_s(info.UTF8SaveTitle,"A_B_World");
    save.LoadSaveData(&info,loaded,nullptr);
    assert(result && save.m_uiSaveSize == 4 && memcmp(save.m_pSaveData,"good",4)==0);

    const char* path = "Windows64\\GameHDD\\test\\A_B_World.ms";
    HANDLE locked = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    assert(locked != INVALID_HANDLE_VALUE);
    memcpy(save.m_pSaveData,"next",4);
    result = true;
    save.SaveSaveData(saved,nullptr);
    assert(!result);
    char original[4] = {};
    DWORD read = 0;
    assert(ReadFile(locked,original,4,&read,nullptr) && read==4 && memcmp(original,"good",4)==0);
    CloseHandle(locked);
    assert(GetFileAttributesA("Windows64\\GameHDD\\test\\A_B_World.ms.tmp")==INVALID_FILE_ATTRIBUTES);

    save.SaveSaveData(saved,nullptr);
    assert(result);
    save.LoadSaveData(&info,loaded,nullptr);
    assert(result && memcmp(save.m_pSaveData,"next",4)==0);
    save.m_bIsSafeDisabled = true;
    save.SaveSaveData(saved,nullptr);
    assert(!result);
    save.m_bIsSafeDisabled = false;
    save.SaveSaveData(nullptr,nullptr); // optional callback

    WIN32_FIND_DATAA entry = {};
    strcpy_s(entry.cFileName,"World.ms.tmp");
    assert(!IsWorldFile(entry));
    strcpy_s(entry.cFileName,"World.ms");
    assert(IsWorldFile(entry));
    entry.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
    assert(!IsWorldFile(entry));

    strcpy_s(info.UTF8SaveTitle,"missing");
    save.LoadSaveData(&info,loaded,nullptr);
    assert(!result && save.m_pSaveData==nullptr && save.m_uiSaveSize==0);

    // Missing parent directories were created by the first save; a file in
    // their place must fail honestly instead of reporting a successful save.
    assert(MoveFileA("Windows64\\GameHDD","Windows64\\GameHDD-backup"));
    HANDLE blocker=CreateFileA("Windows64\\GameHDD",GENERIC_WRITE,0,nullptr,CREATE_NEW,0,nullptr);
    assert(blocker!=INVALID_HANDLE_VALUE);
    CloseHandle(blocker);
    save.m_pSaveData=malloc(4); save.m_uiSaveSize=4;
    memcpy(save.m_pSaveData,"data",4);
    save.SaveSaveData(saved,nullptr);
    assert(!result);
    save.ResetSaveData();
    puts("PASS: save/reload, replacement, rejected commit preserves prior save, invalid directory, missing load, filtering and callback handling");
}
'''

def main():
    source = SOURCE.read_text(encoding="utf-8")
    helper_start = source.index("namespace\n{")
    helper_end = source.index("// @Patoke add", helper_start)
    signatures = [
        "void CSaveGame::ResetSaveData()",
        "void CSaveGame::SetSaveUniqueFilename(",
        "C4JStorage::ESaveGameState CSaveGame::SaveSaveData(",
        "C4JStorage::ESaveGameState CSaveGame::LoadSaveData(PSAVE_INFO",
    ]
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        cpp = folder / "storage.cpp"
        cpp.write_text(STUB + source[helper_start:helper_end] +
                       "\n".join(method(source, s) for s in signatures) + TEST, encoding="utf-8")
        exe = folder / "storage.exe"
        compiler = shutil.which("cl")
        if not compiler:
            raise RuntimeError("Run in an MSVC developer shell on Windows")
        subprocess.run([compiler,"/nologo","/EHsc","/std:c++17","/UNDEBUG",str(cpp),"/Fe:"+str(exe)],cwd=folder,check=True)
        subprocess.run([str(exe)],cwd=folder,check=True)

if __name__ == "__main__":
    main()
