#ifndef CEFFEO_CORE_H
#define CEFFEO_CORE_H

#include "stddef.h" // it was used for size_t


#ifdef __cplusplus
extern "C" {
#endif

    // structure of buffer
    typedef struct {
        char *data; // the text of buffer
        size_t size; // the size of buffer

    } Buffer;

    // Functions 
    void initBuffer(Buffer *buffer);
    void freeBuffer(Buffer *buffer);

    // typedef struct editor {
    //     char buffer[1024 * 16]; // áóôåð òåêñòà
    // } Editor;

    typedef struct editor {
        Buffer buffer;
        char filePath[260];  // Используем массив для пути
    } Editor;


    // Functions 
    void initEditor(Editor *editor); // i need to find the difference between an editor and a buffer.
    
    // Editor* initEditor(void);

    void freeEditor(Editor* ed); // because of the naming when we kill editor it's actually a buffer
                                    // maybe we need to make if there is no buffer kill the editor?

#ifdef __cplusplus
}
#endif

#endif
