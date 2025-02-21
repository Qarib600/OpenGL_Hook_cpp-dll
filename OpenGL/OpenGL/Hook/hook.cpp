#include "hook.h"
#include "MinHook.h"
#include <stdio.h>

Hook::PFN_wglSwapBuffers Hook::original_wglSwapBuffers = nullptr;

namespace Hook {
    FILE* logFile = nullptr;
    HANDLE hConsole = nullptr;
    FARPROC wglSwapBuffersTarget = nullptr;

    void InitConsole() {
        AllocConsole();
        freopen_s(&logFile, "CONOUT$", "w", stdout);
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) {
            MessageBoxA(NULL, "Failed to get console handle!", "Error", MB_OK | MB_ICONERROR);
        }
        SetConsoleTitleA("OpenGL Hook Debug");
    }

    void DebugPrint(const char* message) {
        if (hConsole != nullptr) {
            DWORD written;
            WriteConsoleA(hConsole, message, (DWORD)strlen(message), &written, NULL);
            WriteConsoleA(hConsole, "\n", 1, &written, NULL);
        }
        if (logFile) {
            fprintf(logFile, "%s\n", message);
            fflush(logFile);
        }
    }

    void Draw(HDC hdc) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(50, 50);
        glVertex2f(100, 50);
        glVertex2f(100, 100);
        glVertex2f(50, 100);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();

        DebugPrint("Frame Hooked!");
    }

    BOOL WINAPI Hooked_wglSwapBuffers(HDC hdc) {
        Draw(hdc);
        return original_wglSwapBuffers(hdc);
    }

    static FARPROC ResolveOpenGLFunction(const char* funcName) {
        HMODULE opengl32 = GetModuleHandleA("opengl32.dll");
        if (!opengl32) {
            DebugPrint("opengl32.dll not found in process, attempting to load...");
            opengl32 = LoadLibraryA("opengl32.dll");
            if (!opengl32) {
                char buffer[100];
                sprintf_s(buffer, "Failed to load opengl32.dll, error code: %d", GetLastError());
                DebugPrint(buffer);
                return nullptr;
            }
            DebugPrint("Loaded opengl32.dll manually");
        }
        FARPROC addr = GetProcAddress(opengl32, funcName);
        if (!addr) {
            char buffer[100];
            sprintf_s(buffer, "GetProcAddress failed for %s, error code: %d", funcName, GetLastError());
            DebugPrint(buffer);
            return nullptr;
        }
        if ((uintptr_t)addr < 0x1000) {
            char buffer[100];
            sprintf_s(buffer, "Invalid address for %s: %p", funcName, addr);
            DebugPrint(buffer);
            return nullptr;
        }
        return addr;
    }

    void Initialize() {
        InitConsole();
        fopen_s(&logFile, "C:\\hook_log.txt", "w");

        MH_STATUS initStatus = MH_Initialize();
        if (initStatus != MH_OK) {
            char buffer[100];
            sprintf_s(buffer, "MinHook init failed with code: %d", initStatus);
            DebugPrint(buffer);
            return;
        }

        wglSwapBuffersTarget = ResolveOpenGLFunction("wglSwapBuffers");
        if (!wglSwapBuffersTarget) {
            DebugPrint("Failed to resolve wglSwapBuffers");
            return;
        }

        char addrBuffer[100];
        sprintf_s(addrBuffer, "wglSwapBuffers address: %p", (void*)wglSwapBuffersTarget);
        DebugPrint(addrBuffer);

        MH_STATUS createStatus = MH_CreateHook((LPVOID)wglSwapBuffersTarget, Hooked_wglSwapBuffers,
            (LPVOID*)&original_wglSwapBuffers);
        if (createStatus != MH_OK) {
            char buffer[100];
            sprintf_s(buffer, "Hook creation failed with code: %d", createStatus);
            DebugPrint(buffer);
            return;
        }
        DebugPrint("Hook created successfully");

        MH_STATUS enableStatus = MH_EnableHook((LPVOID)wglSwapBuffersTarget);
        if (enableStatus != MH_OK) {
            char buffer[100];
            sprintf_s(buffer, "Hook enable failed with code: %d", enableStatus);
            DebugPrint(buffer);
            return;
        }

        DebugPrint("Hook installed successfully! Hooray!");
    }

    void Cleanup() {
        MH_DisableHook((LPVOID)wglSwapBuffersTarget);
        MH_Uninitialize();
        if (logFile) {
            fclose(logFile);
            logFile = nullptr;
        }
        if (hConsole != nullptr) {
            FreeConsole();
            hConsole = nullptr;
        }
    }
}