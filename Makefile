P=levelfs
CC=clang

CFLAGS=-Wall -O0 -g
CFLAGS+=-D_FILE_OFFSET_BITS=64
LDLIBS=`pkg-config fuse --cflags --libs`
LDLIBS+=-L./deps/leveldb/ -lleveldb

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

$(P): $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDLIBS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

test:
	$(CC) -DNO_MAIN $(CFLAGS) -Wno-unused-function -Wno-unused-variable $(LDLIBS) $(SRC) test/test.c -o test/test
	time test/test
	rm test/test

tags: $(SRC)
	ctags -R *

clean:
	rm $(P) $(OBJ)

.PHONY: clean test
