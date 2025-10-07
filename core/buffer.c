// this is made for define how buffers work.

// editor.c


#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // Для использования snprintf

// Инициализация редактора
void initEditor(Editor* editor) {
    // Allocate memory for the buffer and initialize it
    editor->buffer.size = 1024;  // Set a default size for the buffer
    editor->buffer.data = (char*)malloc(editor->buffer.size);  // Allocate memory for the buffer
    if (editor->buffer.data == NULL) {
        printf("Failed to allocate buffer memory!\n");
        return;  // Handle memory allocation failure
    }
    memset(editor->buffer.data, 0, editor->buffer.size);  // Initialize the buffer with zeroes
}

// Освобождение редактора
void freeEditor(Editor* editor) {
    freeBuffer(&editor->buffer);
}

// Инициализация буфера
void initBuffer(Buffer* buffer) {
    buffer->data = (char*)malloc(1024);
    if (buffer->data == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    printf("Memory allocated, address: %p\n", buffer->data);

    // Заполнение памяти нулями
    memset(buffer->data, 0, 1024);

    buffer->size = 0;  // Инициализация размера буфера

    // Завершающий нулевой символ
    buffer->data[0] = '\0';  // Гарантируем, что строка начинается с нулевого символа
}

// Освобождение буфера
void freeBuffer(Buffer* buffer) {
    free(buffer->data);
}




// выделение памяти. но надо будет сделать так чтобы выделялась не 1024 символов а больше может 10 мигабайт?