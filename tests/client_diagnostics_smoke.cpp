// Windows-only integration check; no GPU, game assets, or game process required.
#include "../Minecraft.Client/Windows64/ClientDiagnostics.h"
#include <string>
#include <fstream>
#include <iterator>

static bool Contains(const wchar_t* pattern, const char* needle)
{
    WIN32_FIND_DATAW entry;
    HANDLE search = FindFirstFileW(pattern, &entry);
    if (search == INVALID_HANDLE_VALUE) return false;
    bool found = false;
    do
    {
        const std::wstring path = std::wstring(L"logs\\") + entry.cFileName;
        std::ifstream input(path.c_str(), std::ios::binary);
        const std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (contents.find(needle) != std::string::npos) found = true;
    } while (FindNextFileW(search, &entry));
    FindClose(search);
    return found;
}

int main(int argc, char** argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    if (argc == 2 && std::strcmp(argv[1], "--crash") == 0)
    {
        ClientDiagnostics::Initialize();
        ClientDiagnostics::Log("child-before-controlled-crash");
        RaiseException(0xE0424242, EXCEPTION_NONCONTINUABLE, 0, nullptr);
        return 99;
    }
    wchar_t executable[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
    if (!length || length >= MAX_PATH) return 1;
    CreateDirectoryW(L"logs", nullptr);
    CreateDirectoryW(L"logs\\diagnostics-smoke", nullptr);
    if (!SetCurrentDirectoryW(L"logs\\diagnostics-smoke")) return 2;
    ClientDiagnostics::Initialize();
    ClientDiagnostics::Log("diagnostics-message-marker");
    std::printf("stdout-marker\n");
    std::fprintf(stderr, "stderr-marker\n");
    if (!Contains(L"logs\\*.log", "diagnostics-message-marker") ||
        !Contains(L"logs\\*.log", "stdout-marker") ||
        !Contains(L"logs\\*.log", "stderr-marker")) return 3;

    std::wstring command = L"\"" + std::wstring(executable) + L"\" --crash";
    STARTUPINFOW startup = {};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process = {};
    if (!CreateProcessW(executable, &command[0], nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) return 4;
    const DWORD wait = WaitForSingleObject(process.hProcess, 30000);
    if (wait != WAIT_OBJECT_0)
    {
        TerminateProcess(process.hProcess, 98);
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return 5;
    }
    DWORD exitCode = 0;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (exitCode != 0xE0424242 || !Contains(L"logs\\*-crash.txt", "E0424242")) return 6;
    ClientDiagnostics::Log("PASS: debug/stdout/stderr capture and controlled child crash record");
    return 0;
}
