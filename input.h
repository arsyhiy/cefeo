
#pragma once

namespace Input {
    enum KeyType {
        KEY_NONE,
        KEY_CHAR,
        KEY_ENTER,
        KEY_BACKSPACE,
        KEY_ARROW_UP,
        KEY_ARROW_DOWN,
        KEY_ARROW_LEFT,
        KEY_ARROW_RIGHT,
        KEY_ESC,
        KEY_CTRL_C,
        KEY_OTHER
    };

    struct Key {
        KeyType type;
        char ch;
    };

    Key readKey();
}
