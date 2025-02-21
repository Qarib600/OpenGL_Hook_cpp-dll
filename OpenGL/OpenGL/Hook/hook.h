#pragma once
#include <windows.h>
#include <GL/gl.h>


namespace Hook {
    typedef BOOL(WINAPI* PFN_wglSwapBuffers)(HDC hdc);
    extern PFN_wglSwapBuffers original_wglSwapBuffers;

    void Initialize();
    void Cleanup();
    BOOL WINAPI Hooked_wglSwapBuffers(HDC hdc);
    void Draw(HDC hdc);
}