#include "core.h"
#include <stdlib.h>
#include <string.h>

struct editor {
    char buffer[1024];
};

editor_t* editor_create(void) {
    editor_t* ed = (editor_t*)malloc(sizeof(editor_t));
    ed->buffer[0] = '\0';
    return ed;
}

void editor_destroy(editor_t* ed) {
    free(ed);
}

void editor_insert(editor_t* ed, const char* text) {
    strncat(ed->buffer, text, sizeof(ed->buffer) - strlen(ed->buffer) - 1);
}

void editor_render(editor_t* ed, void (*draw_callback)(int x, int y, const char* str)) {
    draw_callback(0, 0, ed->buffer);
}
