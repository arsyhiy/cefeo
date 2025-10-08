// buffer.c


#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



// Инициализация буфера
void initBuffer(Buffer* buffer) {
    buffer->data = (char*)malloc(1024); // remade to 8kb
    if (buffer->data == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    memset(buffer->data, 0, 1024); // also remade to 8kb
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

