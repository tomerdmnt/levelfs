P=levelfs
CC=clang

CFLAGS=-Wall -O0 -g
CFLAGS+=-D_FILE_OFFSET_BITS=64
LDLIBS=`pkg-config fuse --cflags --libs`
LDLIBS+=-lstdc++

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

LIBLEVELDB=deps/leveldb/libleveldb.a

$(P): $(OBJ) $(LIBLEVELDB)
	$(CC) $^ $(CFLAGS) $(LDLIBS) $(LIBLEVELDB) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBLEVELDB):
	@make --directory=deps/leveldb/

test: test.c test.js

test.c:
	$(CC) -DNO_MAIN $(CFLAGS) -Wno-unused-function -Wno-unused-variable $(LDLIBS) $(SRC) test/test.c -o test/test
	time test/test
	rm test/test

test.js: $(P) test/node_modules
	node ./test/test.js

test/node_modules: test/package.json
	@cd test/ && npm install

tags: $(SRC)
	ctags -R *

install:
	cp ./levelfs /usr/local/bin

clean:
	rm $(P) $(OBJ)

.PHONY: clean test test.js test.c
