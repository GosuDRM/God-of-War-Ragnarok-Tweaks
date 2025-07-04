#include "stdafx.h"
#include "helper.hpp"
#include "memory.hpp"

HMODULE baseModule = GetModuleHandle(NULL);

#define wstr(s) L#s
#define wxstr(s) wstr(s)
#define _PROJECT_LOG_PATH _PROJECT_NAME L".log"

wchar_t exePath[_MAX_PATH] = { 0 };

// INI Variables
bool DisableDoF;
bool DisableVignette;

void ReadConfig(void)
{
    inipp::Ini<wchar_t> ini;
    // Get game name and exe path
    LOG(_PROJECT_NAME " Built: " __TIME__ " @ " __DATE__ "\n");
    LOG(L"Game Name: %s\n", Memory::GetVersionProductName().c_str());
    LOG(L"Game Path: %s\n", exePath);

    // Initialize config
    std::wstring config_path = _PROJECT_NAME L".ini";
    std::wifstream iniFile(config_path);
    if (!iniFile)
    {
        // no ini, lets generate one.
        LOG(L"Failed to load config file.\n");
        std::wstring ini_defaults = L"; God of War Ragnarok: Tweaks by GosuDRM @ Nexus Mods\n"
                                    L"[Settings]\n"
                                    L"DisableDoF = true\n"
                                    L"DisableVignette = true\n";
        std::wofstream iniFile(config_path);
        iniFile << ini_defaults;
        DisableDoF = true;
        DisableVignette = true;
        LOG(L"Created default config file.\n");
    }
    else
    {
        ini.parse(iniFile);
        inipp::get_value(ini.sections[L"Settings"], L"DisableDoF", DisableDoF);
        inipp::get_value(ini.sections[L"Settings"], L"DisableVignette", DisableVignette);
    }

    // Log config parse
    LOG(L"DisableDoF: %s (%i)\n", GetBoolStr(DisableDoF), DisableDoF);
    LOG(L"DisableVignette: %s (%i)\n", GetBoolStr(DisableVignette), DisableVignette);
}

void ApplyDoFPatch(void)
{
    // 00007FF753AA59FE | F341:0F59F2 | mulss xmm6,xmm10  -> 0F57F6    | xorps xmm6, xmm6
    // 00007FF753AA5A03 | F341:0F59DA | mulss xmm3, xmm10 -> F3:0F5EDE | divss xmm3, xmm6
    //                                                    -> 90        | nop
    //                                                    -> 90        | nop
    //                                                    -> 90        | nop
    const unsigned char patch[] = { 0x0F, 0x57, 0xF6, 0xF3, 0x0F, 0x5E, 0xDE, 0x90, 0x90, 0x90 };
    WritePatchPattern(L"F3 41 0F 59 F2 F3 41 0F 59 DA", patch, sizeof(patch), L"Disable DoF", 0);
}

uintptr_t fVignette_Address;
float fVignetteFind;
float fVignetteReplace;

DWORD64 DisableVignetteReturnAddress;
void __attribute__((naked)) DisableVignetteAsm()
{
    __asm
    {
        // original code
        movss xmm0, dword ptr [rbx + 0x68];

        // backup xmm1 value
        push rcx;
        movq rcx, xmm1;

        // now we can check the vignette
        movss xmm1, [fVignetteFind];
        comiss xmm0, xmm1;
        jne do_not_replace;

        movss xmm0, [fVignetteReplace];
        do_not_replace :;    // do nothing

        //restore fp registers
        movq xmm1, rcx;

        // apply vignette
        mov rcx, [fVignette_Address];
        movss dword ptr [rcx], xmm0;

        pop rcx;

        // original code
        movss xmm1, dword ptr [rbx + 0x6C];

        jmp [rip + DisableVignetteReturnAddress];
    }
}

void ApplyVignettePatch(void)
{
    // Load fVignetteFind = 0x3E800000 (0.25)
    const unsigned char iVignetteFind[] = { 0x00, 0x00, 0x80, 0x3E };
    memcpy(&fVignetteFind, iVignetteFind, sizeof(iVignetteFind));
    // Load fVignetteReplace
    fVignetteReplace = 1.0;
    // Find actual vignette address (this is necessary because writepatchpattern_hook requires at least 14 bytes of instructions to work)
    fVignette_Address = ReadLEA32(L"F3 0F 10 43 68 F3 0F 11 05 ?? ?? ?? ?? F3 0F 10 4B 6C", L"test", 5, 5+4, 8);

    // Finally, inject code
    WritePatchPattern_Hook(L"F3 0F 10 43 68 F3 0F 11 05 ?? ?? ?? ?? F3 0F 10 4B 6C", 18, L"Disable Vignette", 0, (void*)&DisableVignetteAsm, &DisableVignetteReturnAddress);
}

DWORD __stdcall Main(void*)
{
    bLoggingEnabled = false;
    wchar_t LogPath[_MAX_PATH] = { 0 };
    wcscpy_s(exePath, _countof(exePath), GetRunningPath(exePath));
    _snwprintf_s(LogPath, _countof(LogPath), _TRUNCATE, L"%s\\%s", exePath, _PROJECT_LOG_PATH);

    // Disable logging as this game loads the ASI twice for some reason.
    // This will otherwise incur into a permission denied error since the log file
    // is not closed yet for the second process.
    //LoggingInit(_PROJECT_NAME, LogPath);
    ReadConfig();

    if (DisableDoF)
        ApplyDoFPatch();
    if (DisableVignette)
        ApplyVignettePatch();

    LOG(L"Shutting down " wstr(fp_log) " file handle.\n");
    //fclose(fp_log);
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CreateThread(NULL, 0, Main, 0, NULL, 0);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
