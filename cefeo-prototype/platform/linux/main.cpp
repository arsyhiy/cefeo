#include <X11/Xlib.h>
#include <cstring>
#include "core/core.h"

editor_t* g_editor;

void draw_callback(int x, int y, const char* str) {
    Display* d = XOpenDisplay(NULL);
    if (!d) return;
    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 400, 200, 1,
                                   BlackPixel(d, s), WhitePixel(d, s));
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);

    GC gc = XCreateGC(d, w, 0, NULL);
    XSetForeground(d, gc, BlackPixel(d, s));

    for(;;) {
        XEvent e;
        XNextEvent(d, &e);
        if (e.type == Expose) {
            XDrawString(d, w, gc, 20 + x*8, 40 + y*16, str, strlen(str));
        }
        if (e.type == KeyPress) break;
    }

    XCloseDisplay(d);
}

int main() {
    g_editor = editor_create();
    editor_insert(g_editor, "Hello from Linux!");

    editor_render(g_editor, draw_callback);
    editor_destroy(g_editor);
    return 0;
}
