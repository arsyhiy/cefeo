#include "core.h"
#include <stdlib.h>
#include <string.h>

editor_t* editor_create(void) {
    editor_t* ed = (editor_t*)malloc(sizeof(editor_t));
    if (ed) memset(ed->buffer, 0, sizeof(ed->buffer));
    return ed;
}

void editor_destroy(editor_t* ed) {
    free(ed);
}

