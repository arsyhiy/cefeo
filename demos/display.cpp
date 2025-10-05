
#include "display.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace Display {
    static struct termios orig_termios;

    void disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        std::cout << "\x1b[?25h"; // показать курсор
    }

    void enableRawMode() {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disableRawMode);

        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

        std::cout << "\x1b[?25l"; // скрыть курсор
    }

    void init() {}

    void clearScreen() {
        std::cout << "\x1b[2J\x1b[H";
    }

    void drawTextColored(int row, int col, const std::string& text, int r, int g, int b) {
        std::cout << "\x1b[" << row << ";" << col << "H";
        std::cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m" << text << "\x1b[0m";
    }

    void flush() {
        std::cout.flush();
    }

    int getRows() {
        struct winsize ws;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        return ws.ws_row;
    }

    int getCols() {
        struct winsize ws;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
        return ws.ws_col;
    }
}

