
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

int
main(int argc, char **argv) {
	size_t klen1, klen2;
	test(path_to_key);
	test(key_to_path);

	char *foobar_key = path_to_key("/foo/bar", &klen1);
	char *foorab_key = path_to_key("/foo/rab", &klen2);
	return 0;
}

