"""Exercise actual binary constructor/reload/getData against a parser fixture.

This isolates buffer ownership from the localization parser. It does not validate
binary formats or translated strings. Run with CXXFLAGS sanitizer options too.
"""
from pathlib import Path
import tempfile
import base_stability_regression as fixture

STUB = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
using PBYTE = unsigned char*;
using DWORD = unsigned long;
using UINT = unsigned int;
using LPCWSTR = const wchar_t*;
struct byteArray {
    unsigned char* data = nullptr;
    unsigned int length = 0;
    byteArray() = default;
    byteArray(unsigned char* p, unsigned int n) : data(p), length(n) {}
};
#include "StringTable.h"
static int parses = 0;
void StringTable::ProcessStringTableData() {
    assert(src.length == 4);
    assert(src.data[0] == 10 && src.data[1] == 20 && src.data[2] == 30 && src.data[3] == 40);
    ++parses;
}
void StringTable::ProcessXmlStringTableData(const std::wstring&) { assert(false); }
'''

TEST = r'''
int main() {
    auto* input = new unsigned char[4]{10,20,30,40};
    StringTable table(input, 4);
    PBYTE retained = nullptr;
    UINT size = 0;
    table.getData(&retained, &size);
    assert(size == 4 && retained != input);
    memset(input, 0, 4);
    delete[] input;
    for (int i = 0; i < 100; ++i) table.ReloadStringTable();
    assert(parses == 101);
    table.getData(&retained, &size);
    assert(size == 4 && retained[3] == 40);
}
'''


def method(source, signature):
    start = source.index(signature)
    end = source.index("{", start) + 1
    depth = 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end] + "\n"


def main():
    source = (fixture.ROOT / "Minecraft.Client/StringTable.cpp").read_text()
    signatures = ["StringTable::StringTable(PBYTE", "void StringTable::ReloadStringTable()",
                  "void StringTable::getData(", "StringTable::~StringTable(void)"]
    with tempfile.TemporaryDirectory() as tmp:
        folder = Path(tmp)
        (folder / "StringTable.h").write_bytes((fixture.ROOT / "Minecraft.Client/StringTable.h").read_bytes())
        fixture.compile_and_run(folder, "string_table_lifetime", STUB + "\n".join(method(source, s) for s in signatures) + TEST)


if __name__ == "__main__":
    main()
