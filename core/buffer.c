// this is made for define how buffers work.

// editor.c


#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Инициализация редактора
void initEditor(Editor* editor) {
    editor->buffer.size = 1024;
    editor->buffer.data = (char*)malloc(editor->buffer.size);
    if (editor->buffer.data == NULL) {
        printf("Failed to allocate buffer memory!\n");
        return;
    }
    memset(editor->buffer.data, 0, editor->buffer.size);
}

// Открытие файла
int openFile(Editor* editor, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", filename);
        return 0;  // Ошибка открытия файла
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Перераспределение памяти для данных файла
    editor->buffer.data = (char*)realloc(editor->buffer.data, fileSize + 1);
    if (editor->buffer.data == NULL) {
        printf("Memory allocation failed while reading file!\n");
        fclose(file);
        return 0;
    }

    fread(editor->buffer.data, 1, fileSize, file);
    editor->buffer.data[fileSize] = '\0';  // Завершающий нулевой символ
    editor->buffer.size = fileSize;

    fclose(file);
    return 1;  // Успех
}

// Сохранение файла
int saveFile(Editor* editor, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Failed to open file for saving: %s\n", filename);
        return 0;  // Ошибка открытия файла
    }

    fwrite(editor->buffer.data, 1, editor->buffer.size, file);
    fclose(file);
    return 1;  // Успех
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
    memset(buffer->data, 0, 1024);
    buffer->size = 0;
    buffer->data[0] = '\0';
}

// Освобождение буфера
void freeBuffer(Buffer* buffer) {
    if (buffer->data != NULL) {
        free(buffer->data);
        buffer->data = NULL;
    }
}





// выделение памяти. но надо будет сделать так чтобы выделялась не 1024 символов а больше может 10 мигабайт?