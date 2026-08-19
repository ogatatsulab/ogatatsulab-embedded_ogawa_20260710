CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
TARGET = console_shooter.exe
SOURCES = main.c game.c input.c render.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(OBJECTS) $(TARGET) 2>nul

run: $(TARGET)
	$(TARGET)