#ifndef CEFFEO_CORE_H
#define CEFFEO_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct editor {
        char buffer[1024 * 16]; // буфер текста
    } editor_t;

    editor_t* editor_create(void);
    void editor_destroy(editor_t* ed);

#ifdef __cplusplus
}
#endif

#endif
