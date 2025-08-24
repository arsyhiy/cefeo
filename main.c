#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_COLS  256

char text[MAX_LINES][MAX_COLS];
int lines_count = 1;

int cursor_x = 0, cursor_y = 0;

char current_filename[256] = "";

void draw_text() {
    clear();
    for (int i = 0; i < lines_count; i++) {
        mvprintw(i, 0, "%s", text[i]);
    }
    move(cursor_y, cursor_x);
    refresh();
}

void load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        // Файл не открылся — начинаем с пустого
        return;
    }

    lines_count = 0;
    while (lines_count < MAX_LINES && fgets(text[lines_count], MAX_COLS, f)) {
        text[lines_count][strcspn(text[lines_count], "\r\n")] = 0;  // Убираем \n и \r
        lines_count++;
    }
    fclose(f);

    if (lines_count == 0) {
        lines_count = 1;
        text[0][0] = 0;
    }
}

void save_file() {
    char filename[256];

    if (strlen(current_filename) == 0) {
        echo();
        mvprintw(LINES - 2, 0, "Введите имя файла для сохранения: ");
        clrtoeol();
        getnstr(filename, sizeof(filename) - 1);
        noecho();

        if (strlen(filename) == 0) {
            mvprintw(LINES - 1, 0, "Отмена сохранения.");
            clrtoeol();
            getch();
            return;
        }

        strncpy(current_filename, filename, sizeof(current_filename) - 1);
        current_filename[sizeof(current_filename) - 1] = '\0';
    }

    FILE *f = fopen(current_filename, "w");
    if (!f) {
        mvprintw(LINES - 1, 0, "Ошибка открытия файла для записи.");
        clrtoeol();
        getch();
        return;
    }

    for (int i = 0; i < lines_count; i++) {
        fprintf(f, "%s\n", text[i]);
    }
    fclose(f);

    mvprintw(LINES - 1, 0, "Файл сохранён: %s", current_filename);
    clrtoeol();
    getch();
}

int main(int argc, char *argv[]) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    memset(text, 0, sizeof(text));
    lines_count = 1;

    if (argc > 1) {
        strncpy(current_filename, argv[1], sizeof(current_filename) - 1);
        current_filename[sizeof(current_filename) - 1] = '\0';
        load_file(current_filename);
    }

    draw_text();

    int ch;
    while ((ch = getch()) != KEY_F(3)) {  // F3 — выход
        if (ch == KEY_F(2)) {  // F2 — сохранить
            save_file();
            draw_text();
            continue;
        }

        switch (ch) {
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                else if (cursor_y > 0) {
                    cursor_y--;
                    cursor_x = strlen(text[cursor_y]);
                }
                break;
            case KEY_RIGHT: {
                int len = strlen(text[cursor_y]);
                if (cursor_x < len) cursor_x++;
                else if (cursor_y + 1 < lines_count) {
                    cursor_y++;
                    cursor_x = 0;
                }
                break;
            }
            case KEY_UP:
                if (cursor_y > 0) {
                    cursor_y--;
                    int len = strlen(text[cursor_y]);
                    if (cursor_x > len) cursor_x = len;
                }
                break;
            case KEY_DOWN:
                if (cursor_y + 1 < lines_count) {
                    cursor_y++;
                    int len = strlen(text[cursor_y]);
                    if (cursor_x > len) cursor_x = len;
                }
                break;
            case KEY_BACKSPACE:
            case 127:
                if (cursor_x > 0) {
                    int len = strlen(text[cursor_y]);
                    memmove(&text[cursor_y][cursor_x - 1], &text[cursor_y][cursor_x], len - cursor_x + 1);
                    cursor_x--;
                } else if (cursor_y > 0) {
                    int prev_len = strlen(text[cursor_y - 1]);
                    if (prev_len + strlen(text[cursor_y]) < MAX_COLS - 1) {
                        strcat(text[cursor_y - 1], text[cursor_y]);
                        for (int i = cursor_y; i < lines_count - 1; i++) {
                            strcpy(text[i], text[i + 1]);
                        }
                        lines_count--;
                        cursor_y--;
                        cursor_x = prev_len;
                    }
                }
                break;
            case '\n':
            case '\r': {
                if (lines_count >= MAX_LINES) break;
                char new_line[MAX_COLS] = {0};
                int len = strlen(text[cursor_y]);
                if (cursor_x < len) {
                    strcpy(new_line, &text[cursor_y][cursor_x]);
                    text[cursor_y][cursor_x] = '\0';
                }
                for (int i = lines_count; i > cursor_y + 1; i--) {
                    strcpy(text[i], text[i - 1]);
                }
                lines_count++;
                strcpy(text[cursor_y + 1], new_line);
                cursor_y++;
                cursor_x = 0;
                break;
            }
            default:
                if (ch >= 32 && ch <= 126) {
                    int len = strlen(text[cursor_y]);
                    if (len < MAX_COLS - 1) {
                        memmove(&text[cursor_y][cursor_x + 1], &text[cursor_y][cursor_x], len - cursor_x + 1);
                        text[cursor_y][cursor_x] = ch;
                        cursor_x++;
                    }
                }
                break;
        }
        draw_text();
    }

    endwin();
    return 0;
}

