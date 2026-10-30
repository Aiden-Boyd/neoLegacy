#pragma once
#include <windows.h>
#include <string>

// Small native ABI; the AOT DLL is part of the client build, not a helper app.
namespace JoinCodeRelay
{
    struct Snapshot { int state = 0; int port = 0; std::wstring text; };
    struct Api
    {
        HMODULE module = nullptr;
        bool attempted = false;
        int (__cdecl *host)(int) = nullptr;
        int (__cdecl *join)(const wchar_t*) = nullptr;
        int (__cdecl *status)(wchar_t*, int, int*) = nullptr;
        void (__cdecl *stop)() = nullptr;
        bool Load()
        {
            if (attempted) return module != nullptr;
            attempted = true;
            wchar_t path[MAX_PATH] = {};
            if (!GetModuleFileNameW(nullptr, path, MAX_PATH)) return false;
            std::wstring full(path);
            full = full.substr(0, full.find_last_of(L"\\/" ) + 1) + L"LegacyRelay.dll";
            module = LoadLibraryExW(full.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
            if (!module) return false;
            host = reinterpret_cast<decltype(host)>(GetProcAddress(module, "legacy_host"));
            join = reinterpret_cast<decltype(join)>(GetProcAddress(module, "legacy_join"));
            status = reinterpret_cast<decltype(status)>(GetProcAddress(module, "legacy_status"));
            stop = reinterpret_cast<decltype(stop)>(GetProcAddress(module, "legacy_stop"));
            if (!host || !join || !status || !stop)
            { FreeLibrary(module); module = nullptr; return false; }
            // NativeAOT libraries must stay loaded until process exit.
            return true;
        }
    };
    inline Api& Instance() { static Api api; return api; }
    inline Snapshot Get()
    {
        auto& api = Instance(); Snapshot result;
        if (!api.Load()) { result.state = -1; result.text = L"Relay DLL missing / unsupported"; return result; }
        wchar_t text[256] = {};
        result.state = api.status(text, 256, &result.port); result.text = text;
        return result;
    }
    inline void Host(int port) { auto& api = Instance(); if (api.Load()) api.host(port); }
    inline void Join(const wchar_t* code) { auto& api = Instance(); if (api.Load()) api.join(code); }
    inline void Stop() { auto& api = Instance(); if (api.module && api.stop) api.stop(); }
}
