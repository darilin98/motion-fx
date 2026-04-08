//
// Created by Darek on 28.03.2026.
//

/**
 * @file windllload.cpp
 * @brief Contains a hook for rewiring the included dlls on the Windows platform.
 */

#if defined(_WIN32)
#include <windows.h>
#include <delayimp.h>
#include <string>

static std::wstring getFrameworksDir() {
    wchar_t path[MAX_PATH];
    HMODULE hm = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&getFrameworksDir), &hm);
    GetModuleFileNameW(hm, path, MAX_PATH);

    std::wstring dir(path);
    dir = dir.substr(0, dir.rfind(L'\\'));  // → x86_64-win
    dir = dir.substr(0, dir.rfind(L'\\'));  // → Contents
    dir = dir.substr(0, dir.rfind(L'\\'));  // → bundle root
    dir += L"\\Frameworks";
    return dir;
}

static FARPROC WINAPI delayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli) {
    if (dliNotify == dliNotePreLoadLibrary) {
        static std::wstring frameworksDir = getFrameworksDir();

        // Set search path so transitive deps are also found in Frameworks
        SetDllDirectoryW(frameworksDir.c_str());

        std::wstring fullPath = frameworksDir + L"\\" +
            std::wstring(pdli->szDll, pdli->szDll + strlen(pdli->szDll));

        HMODULE hLib = LoadLibraryW(fullPath.c_str());

        // Reset to default after load to avoid side effects
        SetDllDirectoryW(nullptr);

        if (hLib)
            return reinterpret_cast<FARPROC>(hLib);
    }
    return nullptr;
}

extern "C" const PfnDliHook __pfnDliNotifyHook2 = delayLoadHook;
#endif