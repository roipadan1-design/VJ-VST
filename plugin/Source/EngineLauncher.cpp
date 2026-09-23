#include "EngineLauncher.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <string>

namespace
{
    std::wstring widen (const char* utf8)
    {
        auto length = MultiByteToWideChar (CP_UTF8, 0, utf8, -1, nullptr, 0);
        std::wstring out ((size_t) (length > 0 ? length - 1 : 0), L'\0');
        if (length > 1)
            MultiByteToWideChar (CP_UTF8, 0, utf8, -1, out.data(), length);
        return out;
    }

    HWND findEngineWindow() { return FindWindowW (nullptr, L"VJ Engine"); }
}

bool EngineLauncher::isEngineWindowOpen()
{
    return findEngineWindow() != nullptr;
}

bool EngineLauncher::bringEngineToFront()
{
    auto window = findEngineWindow();
    if (window == nullptr)
        return false;

    if (IsIconic (window))
        ShowWindow (window, SW_RESTORE);

    // The click happened in our (foreground) process, so Windows allows us to
    // hand the foreground to the engine; AllowSetForegroundWindow covers the
    // case where Live's own window currently owns it.
    DWORD processId = 0;
    GetWindowThreadProcessId (window, &processId);
    AllowSetForegroundWindow (processId);
    BringWindowToTop (window);
    return SetForegroundWindow (window) != 0;
}

bool EngineLauncher::launchEngine (const char* exePathUtf8)
{
    auto exe = widen (exePathUtf8);
    auto folder = exe.substr (0, exe.find_last_of (L"\\/"));
    auto result = (INT_PTR) ShellExecuteW (nullptr, L"open", exe.c_str(), nullptr, folder.c_str(), SW_SHOWNORMAL);
    return result > 32;
}
