CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses
TARGET = cefeo
SRC = main.c
GUI = gui-editor
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(GUI)

gui:
	gcc simple_editor_gui.c -o gui-editor `pkg-config --cflags --libs gtksourceview-4`                                    

#debug:
	# i need add a version os this programm to debug it or more clearly a convinient way to debug.
