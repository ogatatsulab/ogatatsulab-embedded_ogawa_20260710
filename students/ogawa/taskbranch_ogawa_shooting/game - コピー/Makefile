CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
LDFLAGS = -lws2_32

TARGET = shooting_game.exe
SOURCES = main.c game.c input.c render.c timer.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul || true

run: $(TARGET)
	$(TARGET)

.DEFAULT_GOAL := all
