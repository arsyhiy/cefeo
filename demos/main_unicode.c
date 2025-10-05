
#define _XOPEN_SOURCE_EXTENDED 1
#include <locale.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define MAX_LINES 1000
#define MAX_LINE_LEN 1024

static wchar_t buffer[MAX_LINES][MAX_LINE_LEN];
static int num_lines = 1;

static int cursor_x = 0;
static int cursor_y = 0;

lua_State *L = NULL;

enum { MODE_EDIT, MODE_COMMAND } mode = MODE_EDIT;

// UTF-8 → wchar_t
void utf8_to_wchar(const char *src, wchar_t *dst, size_t maxlen) {
    mbstate_t state;
    memset(&state, 0, sizeof state);
    size_t len = mbsrtowcs(dst, &src, maxlen - 1, &state);
    if (len == (size_t)-1) dst[0] = L'\0';
    else dst[len] = L'\0';
}

// wchar_t → UTF-8
void wchar_to_utf8(const wchar_t *src, char *dst, size_t maxlen) {
    mbstate_t state;
    memset(&state, 0, sizeof state);
    size_t len = wcsrtombs(dst, &src, maxlen - 1, &state);
    if (len == (size_t)-1) dst[0] = '\0';
    else dst[len] = '\0';
}

void draw() {
    clear();
    for (int i = 0; i < num_lines; i++) {
        mvaddwstr(i, 0, buffer[i]);
    }
    move(cursor_y, cursor_x);

    move(LINES - 1, 0);
    clrtoeol();
    if (mode == MODE_EDIT) {
        printw("-- Редактирование -- ESC: командный режим");
    } else {
        printw("-- Командный режим -- i: редактировать, q: выход");
    }
    refresh();
}

// Lua API: получить курсор
static int l_get_cursor(lua_State *L) {
    lua_pushinteger(L, cursor_x + 1);
    lua_pushinteger(L, cursor_y + 1);
    return 2;
}

// Lua API: установить курсор
static int l_set_cursor(lua_State *L) {
    int x = luaL_checkinteger(L, 1) - 1;
    int y = luaL_checkinteger(L, 2) - 1;
    if (x >= 0 && x < MAX_LINE_LEN) cursor_x = x;
    if (y >= 0 && y < num_lines) cursor_y = y;
    return 0;
}

// Lua API: получить строку
static int l_get_line(lua_State *L) {
    int line = luaL_checkinteger(L, 1) - 1;
    if (line < 0 || line >= num_lines) {
        lua_pushnil(L);
    } else {
        char utf8line[MAX_LINE_LEN * 4];
        wchar_to_utf8(buffer[line], utf8line, sizeof utf8line);
        lua_pushstring(L, utf8line);
    }
    return 1;
}

// Lua API: установить строку
static int l_set_line(lua_State *L) {
    int line = luaL_checkinteger(L, 1) - 1;
    const char *text = luaL_checkstring(L, 2);
    if (line >= 0 && line < MAX_LINES) {
        utf8_to_wchar(text, buffer[line], MAX_LINE_LEN);
        if (line >= num_lines) num_lines = line + 1;
    }
    return 0;
}

void init_lua() {
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "get_cursor", l_get_cursor);
    lua_register(L, "set_cursor", l_set_cursor);
    lua_register(L, "get_line", l_get_line);
    lua_register(L, "set_line", l_set_line);
}

void run_command() {
    echo();
    char cmd[256];
    mvprintw(LINES - 1, 0, ":");
    getnstr(cmd, sizeof(cmd) - 1);
    noecho();

    if (strncmp(cmd, "lua ", 4) == 0) {
        if (luaL_dostring(L, cmd + 4)) {
            mvprintw(LINES - 2, 0, "Ошибка Lua: %s", lua_tostring(L, -1));
            lua_pop(L, 1);
            getch();
        }
    }
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    init_lua();

    while (1) {
        draw();
        int ch = getch();

        if (mode == MODE_EDIT) {
            if (ch == 27) { // ESC
                mode = MODE_COMMAND;
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (cursor_x > 0) {
                    cursor_x--;
                    buffer[cursor_y][cursor_x] = L'\0';
                }
            } else if (ch == '\n') {
                if (num_lines < MAX_LINES - 1) {
                    for (int i = num_lines; i > cursor_y + 1; i--) {
                        wcscpy(buffer[i], buffer[i - 1]);
                    }
                    buffer[cursor_y + 1][0] = L'\0';
                    num_lines++;
                    cursor_y++;
                    cursor_x = 0;
                }
            } else if (ch >= 32 && ch <= 126) {
                if (cursor_x < MAX_LINE_LEN - 1) {
                    buffer[cursor_y][cursor_x++] = ch;
                    buffer[cursor_y][cursor_x] = L'\0';
                }
            }
        } else if (mode == MODE_COMMAND) {
            if (ch == 'i') {
                mode = MODE_EDIT;
            } else if (ch == 'q') {
                break;
            } else if (ch == ':') {
                run_command();
            }
        }

        if (ch == KEY_UP && cursor_y > 0) cursor_y--;
        if (ch == KEY_DOWN && cursor_y < num_lines - 1) cursor_y++;
        if (ch == KEY_LEFT && cursor_x > 0) cursor_x--;
        if (ch == KEY_RIGHT && cursor_x < wcslen(buffer[cursor_y])) cursor_x++;
    }

    endwin();
    lua_close(L);
    return 0;
}

