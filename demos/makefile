CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses
TARGET = cefeo
SRC = main.c
GUI = gui-editor
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

terminal_unicode:
	gcc -Wall -Wextra -O2 -o cefeo main_unicode.c -I/usr/include/lua5.3 -lncursesw -llua5.3 -lm

clean:
	rm -f $(TARGET) $(GUI) main_unicode 

gui:
	gcc simple_editor_gui.c -o gui-editor `pkg-config --cflags --libs gtksourceview-4`                                    

#debug:
	# i need add a version os this programm to debug it or more clearly a convinient way to debug.
