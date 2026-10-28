#pragma once

// Header-only so the legacy client source lists need no extra translation unit.
#include <windows.h>
#include <dbghelp.h>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <io.h>

namespace ClientDiagnostics
{
inline HANDLE crashFile = INVALID_HANDLE_VALUE;
inline bool ready = false;
inline wchar_t dumpPath[MAX_PATH] = {};
inline LONG crashing = 0;
inline LPTOP_LEVEL_EXCEPTION_FILTER previousFilter = nullptr;
using DumpWriter = BOOL (WINAPI *)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
    PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION,
    PMINIDUMP_CALLBACK_INFORMATION);
inline DumpWriter writeDump = nullptr;

inline void Log(const char* message)
{
    // Global constructors can emit debug messages before the entry point.
    if (!ready) { OutputDebugStringA(message); return; }
    SYSTEMTIME now;
    GetLocalTime(&now);
    _lock_file(stderr);
    std::fprintf(stderr, "[%02u:%02u:%02u.%03u tid=%lu] %s",
        now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
        GetCurrentThreadId(), message);
    const size_t length = std::strlen(message);
    if (length == 0 || message[length - 1] != '\n') std::fputc('\n', stderr);
    std::fflush(stderr);
    _unlock_file(stderr);
}

inline LONG WINAPI OnCrash(EXCEPTION_POINTERS* exception)
{
    // Avoid CRT locks/heap work here: another crashing thread may hold them.
    if (InterlockedCompareExchange(&crashing, 1, 0) == 0)
    {
        char record[256];
        const int length = wsprintfA(record,
            "Unhandled exception code=0x%08lX address=%p pid=%lu tid=%lu\r\n",
            exception->ExceptionRecord->ExceptionCode,
            exception->ExceptionRecord->ExceptionAddress,
            GetCurrentProcessId(), GetCurrentThreadId());
        DWORD written = 0;
        if (crashFile != INVALID_HANDLE_VALUE)
        {
            WriteFile(crashFile, record, length, &written, nullptr);
            FlushFileBuffers(crashFile);
        }
        if (writeDump && dumpPath[0])
        {
            HANDLE dump = CreateFileW(dumpPath, GENERIC_WRITE, FILE_SHARE_READ,
                nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (dump != INVALID_HANDLE_VALUE)
            {
                MINIDUMP_EXCEPTION_INFORMATION info = {GetCurrentThreadId(), exception, FALSE};
                const BOOL ok = writeDump(GetCurrentProcess(), GetCurrentProcessId(), dump,
                    static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithThreadInfo),
                    &info, nullptr, nullptr);
                const DWORD error = ok ? ERROR_SUCCESS : GetLastError();
                CloseHandle(dump);
                const int resultLength = wsprintfA(record, "Minidump success=%lu error=%lu\r\n", static_cast<DWORD>(ok), error);
                if (crashFile != INVALID_HANDLE_VALUE)
                {
                    WriteFile(crashFile, record, resultLength, &written, nullptr);
                    FlushFileBuffers(crashFile);
                }
            }
        }
    }
    return previousFilter ? previousFilter(exception) : EXCEPTION_CONTINUE_SEARCH;
}

inline void Initialize()
{
    // Prefer the game folder. A read-only install falls back to the user's temp.
    wchar_t folder[MAX_PATH] = L"logs";
    CreateDirectoryW(folder, nullptr);
    SYSTEMTIME now;
    GetLocalTime(&now);
    wchar_t stem[96];
    swprintf_s(stem, L"client-%04u%02u%02u-%02u%02u%02u-%lu",
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond,
        GetCurrentProcessId());
    wchar_t logPath[MAX_PATH];
    swprintf_s(logPath, L"%s\\%s.log", folder, stem);
    FILE* opened = nullptr;
    if (_wfreopen_s(&opened, logPath, L"w", stderr) != 0)
    {
        const DWORD length = GetTempPathW(MAX_PATH, folder);
        if (length == 0 || length >= MAX_PATH - 120) return;
        wcscat_s(folder, L"neoLegacy-logs");
        CreateDirectoryW(folder, nullptr);
        swprintf_s(logPath, L"%s\\%s.log", folder, stem);
        if (_wfreopen_s(&opened, logPath, L"w", stderr) != 0) return;
    }
    setvbuf(stderr, nullptr, _IONBF, 0);
    if (_dup2(_fileno(stderr), _fileno(stdout)) == 0)
        setvbuf(stdout, nullptr, _IONBF, 0);
    wchar_t crashPath[MAX_PATH];
    swprintf_s(crashPath, L"%s\\%s-crash.txt", folder, stem);
    crashFile = CreateFileW(crashPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    swprintf_s(dumpPath, L"%s\\%s.dmp", folder, stem);
    // Load before a crash; never add a mandatory DLL dependency for diagnostics.
    HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll");
    if (dbghelp) writeDump = reinterpret_cast<DumpWriter>(GetProcAddress(dbghelp, "MiniDumpWriteDump"));
    previousFilter = SetUnhandledExceptionFilter(OnCrash);
    ready = true;
    Log("Diagnostics started; stdout/stderr and client debug messages captured.");
    std::fprintf(stderr, "Runtime log: %ls\nCrash record: %ls\nMinidump available: %s\n",
        logPath, crashPath, writeDump ? "yes" : "no");
}
}
