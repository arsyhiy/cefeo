#include <iostream>
#include <vector>
#include <string>
#include <cwchar>
#include <clocale>
#include <ncursesw/ncurses.h>
#include <lua.hpp>

class Editor {
public:
    static constexpr int MAX_LINES = 1024;
    static constexpr int MAX_LINE_LEN = 1024;

    Editor() : cursor_x(0), cursor_y(0), mode(0), cmd_pos(0) {}

    void init() {
        setlocale(LC_ALL, "");
        initscr();
        raw();
        keypad(stdscr, TRUE);
        noecho();
        if (lines.empty()) lines.push_back(L"");
    }

    void fini() {
        endwin();
    }

    void loadFile(const std::string &filename) {
        FILE *fp = fopen(filename.c_str(), "r");
        if (!fp) return;
        wchar_t line[MAX_LINE_LEN];
        lines.clear();
        while (fgetws(line, MAX_LINE_LEN, fp) && (int)lines.size() < MAX_LINES) {
            size_t len = wcslen(line);
            if (len > 0 && line[len - 1] == L'\n') line[len - 1] = L'\0';
            lines.emplace_back(line);
        }
        fclose(fp);
        if (lines.empty()) lines.push_back(L"");
    }

    void saveFile(const std::string &filename) {
        FILE *fp = fopen(filename.c_str(), "w");
        if (!fp) return;
        for (auto &line : lines) {
            fwprintf(fp, L"%ls\n", line.c_str());
        }
        fclose(fp);
    }

    void run() {
        draw();
        while (true) {
            wint_t ch;
            get_wch(&ch);
            if (mode == 0) handleInsert(ch);
            else handleCommand(ch);
            draw();
        }
    }

private:
    std::vector<std::wstring> lines;
    int cursor_x, cursor_y;
    int mode; // 0 insert, 1 command
    std::wstring command;
    int cmd_pos;

    void draw() {
        clear();
        for (int i = 0; i < (int)lines.size(); i++) {
            mvaddwstr(i, 0, lines[i].c_str());
        }
        if (mode == 1) {
            mvprintw(LINES - 1, 0, ":%ls", command.c_str());
            move(LINES - 1, cmd_pos + 1);
        } else {
            move(cursor_y, cursor_x);
        }
        refresh();
    }

    void insertChar(wchar_t ch) {
        std::wstring &line = lines[cursor_y];
        if ((int)line.size() >= MAX_LINE_LEN - 1) return;
        line.insert(line.begin() + cursor_x, ch);
        cursor_x++;
    }

    void deleteChar() {
        std::wstring &line = lines[cursor_y];
        if (cursor_x > 0 && !line.empty()) {
            line.erase(line.begin() + cursor_x - 1);
            cursor_x--;
        }
    }

    void newLine() {
        if ((int)lines.size() >= MAX_LINES - 1) return;
        std::wstring &line = lines[cursor_y];
        std::wstring newLine = line.substr(cursor_x);
        line.erase(cursor_x);
        lines.insert(lines.begin() + cursor_y + 1, newLine);
        cursor_y++;
        cursor_x = 0;
    }

    void handleInsert(wint_t ch) {
        if (ch == 27) { // ESC
            mode = 1;
            command.clear();
            cmd_pos = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            deleteChar();
        } else if (ch == KEY_LEFT && cursor_x > 0) {
            cursor_x--;
        } else if (ch == KEY_RIGHT && cursor_x < (int)lines[cursor_y].size()) {
            cursor_x++;
        } else if (ch == KEY_UP && cursor_y > 0) {
            cursor_y--;
            if (cursor_x > (int)lines[cursor_y].size())
                cursor_x = lines[cursor_y].size();
        } else if (ch == KEY_DOWN && cursor_y + 1 < (int)lines.size()) {
            cursor_y++;
            if (cursor_x > (int)lines[cursor_y].size())
                cursor_x = lines[cursor_y].size();
        } else if (ch == '\n') {
            newLine();
        } else if (ch >= 32 && ch <= 126) {
            insertChar(ch);
        }
    }

    void handleCommand(wint_t ch) {
        if (ch == '\n') {
            executeCommand();
            mode = 0;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cmd_pos > 0) {
                command.pop_back();
                cmd_pos--;
            }
        } else if (ch >= 32 && ch < 127 && cmd_pos < 255) {
            command.push_back(ch);
            cmd_pos++;
        }
    }

    void executeCommand() {
        if (command == L"q") {
            fini();
            exit(0);
        } else if (command == L"w") {
            saveFile("output.txt");
        } else if (command.rfind(L"w ", 0) == 0) {
            std::string filename;
            for (size_t i = 2; i < command.size(); i++) {
                filename.push_back((char)command[i]);
            }
            saveFile(filename);
        }
        command.clear();
        cmd_pos = 0;
    }
};

class LuaEngine {
public:
    LuaEngine() {
        L = luaL_newstate();
        luaL_openlibs(L);
    }

    ~LuaEngine() {
        if (L) lua_close(L);
    }

    void runScript(const std::string &file) {
        if (luaL_dofile(L, file.c_str()) != LUA_OK) {
            std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }

private:
    lua_State *L;
};

int main(int argc, char *argv[]) {
    Editor editor;
    editor.init();

    if (argc > 1) {
        editor.loadFile(argv[1]);
    }

    editor.run();
    editor.fini();
    return 0;
}
