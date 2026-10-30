// Exercise the same loader/header compiled into the actual game, from a native
// Windows executable with no managed host or installed-runtime dependency.
#include "../../../Minecraft.Client/Windows64/Network/JoinCodeRelay.h"
#include <iostream>
#include <stdexcept>

static void Require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
int main()
{
    try
    {
        JoinCodeRelay::Stop();
        auto status = JoinCodeRelay::Get();
        Require(status.state == 0 && status.port == 0, "DLL load / idle snapshot failed");
        JoinCodeRelay::Join(L"INVALID");
        Require(JoinCodeRelay::Get().state == -1, "Invalid code accepted");
        JoinCodeRelay::Host(25565);
        const ULONGLONG deadline = GetTickCount64() + 30000;
        do { Sleep(100); status = JoinCodeRelay::Get(); }
        while (status.state == 1 && GetTickCount64() < deadline);
        Require(status.state == 2 && status.text.size() == 11 && status.text[5] == L'-', "Public host registration / code snapshot failed");
        std::cout << "PASS: actual client loader, native ABI, invalid code rejection and public host registration.\n";
        JoinCodeRelay::Stop();
        for (int i = 0; i < 10; ++i)
        {
            JoinCodeRelay::Join(L"ABCDE-FGHJK");
            JoinCodeRelay::Stop();
        }
        Sleep(250);
        status = JoinCodeRelay::Get();
        Require(status.state == 0 && status.port == 0, "Cancelled session revived");
        auto test = reinterpret_cast<int (__cdecl*)()>(GetProcAddress(JoinCodeRelay::Instance().module, "legacy_self_test"));
        Require(test && test() == 0, "Native public transport test failed");
        JoinCodeRelay::Stop();
        std::cout << "PASS: native DLL four-stream 4 MiB exact public TCP echo and cancellation/replacement.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        JoinCodeRelay::Stop();
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
