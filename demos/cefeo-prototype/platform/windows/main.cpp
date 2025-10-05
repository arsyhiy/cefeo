#include <windows.h>
#include "core/core.h"

editor_t* g_editor;

void draw_callback(int x, int y, const char* str) {
    HDC hdc = GetDC(GetConsoleWindow());
    TextOutA(hdc, 10 + x*8, 10 + y*16, str, lstrlenA(str));
    ReleaseDC(GetConsoleWindow(), hdc);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    g_editor = editor_create();
    editor_insert(g_editor, "Hello from Windows!");

    editor_render(g_editor, draw_callback);
    MessageBoxA(NULL, "Text rendered in console window", "Cefeo", MB_OK);

    editor_destroy(g_editor);
    return 0;
}
