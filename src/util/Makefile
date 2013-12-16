CFLAGS = -DNDEBUG -g -O2 -Wall
LFLAGS = 
CC = gcc
INCLUDE_DIR = ../../inc 
SRCS = $(shell ls *.c) 
OBJS = $(SRCS:.c=.o)

all: $(OBJS)

%.o: %.c
	$(CC) -o $@ -c -I$(INCLUDE_DIR) $(CFLAGS) $<

install : 
clean:
	rm -rf $(OBJS) $(BIN)
