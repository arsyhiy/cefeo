
#include "input.h"
#include <unistd.h>

namespace Input {
    Key readKey() {
        char c;
        int n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) return {KEY_NONE, 0};

        if (c == '\x1b') { // ESC-последовательности
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) return {KEY_ESC, 0};
            if (read(STDIN_FILENO, &seq[1], 1) == 0) return {KEY_ESC, 0};

            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': return {KEY_ARROW_UP, 0};
                    case 'B': return {KEY_ARROW_DOWN, 0};
                    case 'C': return {KEY_ARROW_RIGHT, 0};
                    case 'D': return {KEY_ARROW_LEFT, 0};
                }
            }
            return {KEY_OTHER, 0};
        }

        if (c == 127) return {KEY_BACKSPACE, 0};
        if (c == '\r' || c == '\n') return {KEY_ENTER, 0};
        if (c == 3) return {KEY_CTRL_C, 0}; // Ctrl-C
        return {KEY_CHAR, c};
    }
}

