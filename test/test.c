
#include "../src/path.h"
#include "../src/main.c"

#define test(name) { \
	printf("testing %s\t\t", #name);	\
	test_##name();				\
	printf("OK\n");				\
}

void
test_path_to_key() {
	size_t klen;
	assert(strncmp(path_to_key("/foo/bar", 8, &klen), "foo.bar", 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("foo/bar", 7, &klen), "foo.bar", 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("/foo/bar/", 9, &klen), "foo.bar", 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("/foo/b", 6, &klen), "foo.b", 5) == 0);
	assert(klen == 5);
}

void
test_key_to_path() {
	size_t plen;
	assert(strncmp(key_to_path("foo.bar", 7, &plen), "/foo/bar", 8) == 0);
	assert(plen == 8);
	assert(strncmp(key_to_path("foo.bar.", 8, &plen), "/foo/bar/", 9) == 0);
	assert(plen == 9);
	assert(strncmp(key_to_path("foo.b", 5, &plen), "/foo/b", 6) == 0);
	assert(plen == 6);
}

void
db_put(char *key, char *val) {
	char *err;
	leveldb_options_t *opts = leveldb_options_create();
	leveldb_t *db = leveldb_open(opts, "testdb", &err);
	leveldb_writeoptions_t *wopts = leveldb_writeoptions_create();
	leveldb_put(db, wopts, key, strlen(key), val, strlen(val), &err);
	leveldb_close(db);
}

int
main(int argc, char **argv) {
	test(path_to_key);
	test(key_to_path);

	db_put("test_key1", "test val 1");
	db_put("foo.bar", "foo bar");
	db_put("foo.rab", "foo rab");
	return 0;
}

