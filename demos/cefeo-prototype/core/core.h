#ifndef CEFFEO_CORE_H
#define CEFFEO_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct editor editor_t;

editor_t* editor_create(void);
void editor_destroy(editor_t* ed);

void editor_insert(editor_t* ed, const char* text);
void editor_render(editor_t* ed, void (*draw_callback)(int x, int y, const char* str));

#ifdef __cplusplus
}
#endif

#endif


// done.