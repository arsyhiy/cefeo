// editor.c
// Инициализация редактора

#include "editor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initEditor(Editor* editor) {
    editor->buffer.size = 1024; // надо будет переписать как 8кб
    editor->buffer.data = (char*)malloc(editor->buffer.size);
    if (editor->buffer.data == NULL) {
        printf("Failed to allocate buffer memory!\n");
        return;
    }
    memset(editor->buffer.data, 0, editor->buffer.size);
}

// Освобождение редактора
void freeEditor(Editor* editor) {
    freeBuffer(&editor->buffer);
}
