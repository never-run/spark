CFLAGS = -g -O2 -Wall
LFLAGS = -ldl -rdynamic
CC = gcc
INCLUDE_DIR = ../../inc 
BIN  = spark_demo
SRCS = $(shell ls *.c) 
OBJS = $(SRCS:.c=.o)
EXT_OBJS = ../cap_ada/spk_cap_adpt.o

%.o: %.c
	$(CC) -o $@ -c -I$(INCLUDE_DIR) $(CFLAGS) $<

all: $(BIN)

$(BIN) : $(OBJS) $(EXT_OBJS)
	$(CC) $(OBJS) $(EXT_OBJS) -o $(BIN) -I$(INCLUDE_DIR) $(CFLAGS) $(LFLAGS)

install : 
	cp $(BIN) ../../bin

clean:
	rm -rf $(OBJS) $(BIN)
