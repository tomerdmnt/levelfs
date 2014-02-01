
#include <string.h>
#include <stdlib.h>

#include "path.h"
#include "config.h"


const char *delim = ".";

/*
 * /foo/bar -> foo.bar
 */
char *
path_to_key(const char *path, size_t plen, size_t *klen) {
	char *key;

	*klen = plen;
	if (path[plen-1] == '/')
		if (*klen > 0) (*klen)--;
	if (path[0] == '/') {
		path++;
		if (*klen > 0) (*klen)--;
	}
	key = malloc(*klen);

	for (int i = 0; i < *klen; ++i) {
		if (path[i] == '/')
			strncpy(key+i, delim, 1);
		else
			key[i] = path[i];
	}

	return key;
}

/*
 * foo.bar -> /foo/bar
 */
char *
key_to_path(const char *key, size_t klen) {
	char *path;

	path = malloc(klen+2);
	path[0] = '/';

	for (int i = 0; i < klen; ++i) {
		if (strncmp(key+i, delim, 1) == 0)
			path[i+1] = '/';
		else
			path[i+1] = key[i];
	}

	path[klen+1] = '\0';
	return path;
}

