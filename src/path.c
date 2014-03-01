
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "path.h"

static unsigned const char sep = 0xff;

char
sublevel_seperator() {
	return sep;
}

/*
 * /foo/bar -> foo.bar
 */
char *
path_to_key(const char *path, size_t *klen) {
	char *key;
	size_t plen;

	plen = strlen(path);
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
			key[i] = sep;
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
		if (key[i] == sep)
			path[i+1] = '/';
		else
			path[i+1] = key[i];
	}

	path[klen+1] = '\0';
	return path;
}

const char *
path_diff(const char *base_path, const char *path) {
	size_t base_path_len, path_len;

	base_path_len = strlen(base_path);
	path_len = strlen(path);

	if (base_path_len > path_len)
		return NULL;
	if (strncmp(base_path, path, base_path_len) != 0)
		return NULL;
	path += base_path_len;
	if (*path == '/') path++;
	return path;
}

char *
key_append_sep(char *key, size_t *klen) {
	(*klen)++;
	key = realloc(key, *klen);
	key[*klen-1] = sep;
	return key;
}

char *
dirname(const char *path) {
	char *dirname, *p;
	size_t len;
	
	p = strrchr(path, '/');
	if (!p) {
		len = 1;
		path = ".";
	} else if (p == path) {
		len = 1;
		path = "/";
	} else {
		len = p - path;
	}
	dirname = malloc(len+1);
	strncpy(dirname, path, len);
	dirname[len] = '\0';
	return dirname;
}

char *
basename(const char *path) {
	const char *p;

	p = strrchr(path, '/');
	if (!p)
		p = path;
	else
		p++;
	return strdup(p);
}

