CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses
TARGET = cefeo
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

#debug:
	# i need add a version os this programm to debug it or more clearly a convinient way to debug.
