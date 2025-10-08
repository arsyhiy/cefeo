// editor.h


#include "buffer.h"

    //structures
    typedef struct editor {
        Buffer buffer;
        char filePath[260];  // Используем массив для пути. NOTE: а почему 260?
    } Editor;


    // Functions 
    void initEditor(Editor *editor); // i need to find the difference between an editor and a buffer.
    void freeEditor(Editor* ed); // because of the naming when we kill editor it's actually a buffer
                                 // maybe we need to make if there is no buffer kill the editor?

