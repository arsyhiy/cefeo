
#include <ncurses.h>
int main() {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    int ch;
    printw("Нажмите клавишу (ESC чтобы выйти):\n");
    while ((ch = getch()) != 27) {  // ESC = 27
        printw("Код нажатой клавиши: %d\n", ch);
        refresh();
    }

    endwin();
    return 0;
}
