
#include "display.h"
#include "input.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

int main() {
    using namespace Display;
    using namespace Input;

    enableRawMode();
    init();
    clearScreen();

    drawTextColored(1, 2, "Mini TUI demo — press q to quit", 255, 200, 0);
    drawTextColored(3, 2, "Use arrows, Backspace, Enter. Type to see characters.", 180,180,180);
    flush();

    while (true) {
        Key k = readKey();
        if (k.type == KEY_NONE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (k.type == KEY_CHAR) {
            if (k.ch == 'q') break;
            std::string s(1, k.ch);
            drawTextColored(5, 2, "You typed: " + s + "    ", 200,255,200);
            flush();
        } else if (k.type == KEY_ARROW_UP) {
            drawTextColored(6,2,"Arrow: UP   ", 200,200,255); flush();
        } else if (k.type == KEY_ARROW_DOWN) {
            drawTextColored(6,2,"Arrow: DOWN ", 200,200,255); flush();
        } else if (k.type == KEY_ARROW_LEFT) {
            drawTextColored(6,2,"Arrow: LEFT ", 200,200,255); flush();
        } else if (k.type == KEY_ARROW_RIGHT) {
            drawTextColored(6,2,"Arrow: RIGHT", 200,200,255); flush();
        } else if (k.type == KEY_BACKSPACE) {
            drawTextColored(7,2,"BACKSPACE   ", 255,200,200); flush();
        } else if (k.type == KEY_ENTER) {
            drawTextColored(7,2,"ENTER       ", 255,200,200); flush();
        } else if (k.type == KEY_CTRL_C) {
            break;
        }
    }

    disableRawMode();
    std::cout << "\x1b[2J\x1b[H";
    return 0;
}
