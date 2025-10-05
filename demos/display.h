
#pragma once
#include <string>

namespace Display {
    void enableRawMode();
    void disableRawMode();
    void init();
    void clearScreen();
    void drawTextColored(int row, int col, const std::string& text, int r, int g, int b);
    void flush();
    int getRows();
    int getCols();
}
