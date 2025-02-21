#include "../Hook/hook.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        Hook::Initialize();
        break;
    case DLL_PROCESS_DETACH:
        Hook::Cleanup();
        break;
    }
    return TRUE;
}