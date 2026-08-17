CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O3

TARGET = gameboy

SRCS = main.c CPU.c MMU.c instr_handler.c cb_dispatcher.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean