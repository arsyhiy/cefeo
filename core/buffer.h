#ifndef CEFFEO_CORE_H
#define CEFFEO_CORE_H

#include "stddef.h" // it was used for size_t


#ifdef __cplusplus
extern "C" {
#endif

    // structure of buffer
    // i think i need to move structs somewhere  else
    typedef struct {
        char *data; // the text of buffer
        size_t size; // the size of buffer

    } Buffer;

    // Functions 
    void initBuffer(Buffer *buffer);
    void freeBuffer(Buffer *buffer);

 
#ifdef __cplusplus
}
#endif

#endif
