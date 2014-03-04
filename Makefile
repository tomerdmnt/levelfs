P = levelfs
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
	CC=clang
else
	CC=gcc
endif

CFLAGS = -Wall -O0 -g
CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -I/usr/local/include/osxfuse
LDLIBS = `pkg-config fuse --cflags --libs`
LDLIBS += -lstdc++

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

LIBLEVELDB=deps/leveldb/libleveldb.a

$(P): $(LIBLEVELDB) $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDLIBS) $(LIBLEVELDB) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBLEVELDB): deps/leveldb/.git
	@make --directory=deps/leveldb/

deps/leveldb/.git:
	git submodule init
	git submodule update

test: test.js

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
