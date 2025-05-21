CC = gcc
CFLAGS = -Wall -g
TARGET = battleship
SOURCES = battleship.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) fifo1 fifo2 player1_ships.txt player2_ships.txt

.PHONY: all clean