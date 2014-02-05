
#include "../src/path.h"
#include "../src/levelfs.c"

#define test(name) { \
	printf("testing %s\t\t", #name);	\
	test_##name();				\
	printf("OK\n");				\
}

void
test_path_to_key() {
	size_t klen;
	assert(strncmp(path_to_key("/foo/bar", &klen), (char []){'f','o','o',0xff,'b','a','r'}, 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("foo/bar", &klen), (char []){'f','o','o',0xff,'b','a','r'}, 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("/foo/bar/", &klen), (char []){'f','o','o',0xff,'b','a','r'}, 7) == 0);
	assert(klen == 7);
	assert(strncmp(path_to_key("/foo/b", &klen), (char []){'f','o','o',0xff,'b'}, 5) == 0);
	assert(klen == 5);
}

void
test_key_to_path() {
	assert(strncmp(key_to_path((char []){'f','o','o',0xff,'b','a','r'}, 7), "/foo/bar", 8) == 0);
	assert(strncmp(key_to_path((char []){'f','o','o',0xff,'b','a','r',0xff}, 8), "/foo/bar/", 9) == 0);
	assert(strncmp(key_to_path((char []){'f','o','o',0xff,'b'}, 5), "/foo/b", 6) == 0);
}

void
db_put(char *key, char *val, size_t klen) {
	char *err = NULL;
	levelfs_db_t *db = levelfs_db_open("testdb", &err);
	if (err) {
		fprintf(stderr, "leveldb open: %s\n", err);
		exit(1);
	}
	levelfs_db_put(db, key, klen, val, strlen(val), &err);
	if (err) {
		fprintf(stderr, "leveldb put: %s\n", err);
		exit(1);
	}
	levelfs_db_close(db);
}

int
main(int argc, char **argv) {
	size_t klen1, klen2;
	test(path_to_key);
	test(key_to_path);

	char *foobar_key = path_to_key("/foo/bar", &klen1);
	char *foorab_key = path_to_key("/foo/rab", &klen2);
	db_put("test_key1", "test val 1", 9);
	db_put(foobar_key, "foo bar", klen1);
	db_put(foorab_key , "foo rab", klen2);
	return 0;
}

