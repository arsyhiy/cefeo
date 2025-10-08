#include "editor.h"
#include <stdlib.h>
//#include <string.h>
#include <stdio.h>


// Открытие файла
int openFile(Editor* editor, const char* filename) {
    FILE* file = fopen(filename, "r"); // заменить fopen
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
    FILE* file = fopen(filename, "w"); // заменить fopen
    if (file == NULL) {
        printf("Failed to open file for saving: %s\n", filename);
        return 0;  // Ошибка открытия файла
    }

    fwrite(editor->buffer.data, 1, editor->buffer.size, file);
    fclose(file);
    return 1;  // Успех
}
