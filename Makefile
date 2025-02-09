CC = gcc
CFLAGS = -Wall -Wextra -I./include
SRCS = src/main.c src/display.c src/memory.c src/args.c src/utils.c
OBJS = $(SRCS:.c=.o)
TARGET = freed

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)