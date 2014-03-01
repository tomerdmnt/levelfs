
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "path.h"

static int seplen = 2;
static const char sep[] = {0xc3, 0xbf};

int
sepcmp(const char *str, size_t len) {
	if (len < seplen)
		return -1;
	return strncmp(str, &(sep[0]), seplen);
}

/*
 * /foo/bar -> .foo.bar
 */
char *
path_to_key(const char *path, size_t *klen, char appendsep) {
	char *key, *k;
	size_t plen;
	int i;

	plen = strlen(path);
	appendsep = appendsep && path[plen-1] != '/';

	for (i = 0, *klen = 0; i < plen; i++) {
		if (path[i] == '/')
			(*klen) += seplen;
		else
			(*klen)++;
	}
	if (appendsep)
		(*klen) += seplen;
	key = malloc(*klen);

	for (i = 0, k = key; i < plen; ++i) {
		if (path[i] == '/') {
			strncpy(k, &(sep[0]), seplen);
			k += seplen;
		} else {
			*k = path[i];
			k++;
		}
	}
	if (appendsep)
		strncpy(k, &(sep[0]), seplen);

	return key;
}

/*
 * .foo.bar -> /foo/bar
 */
char *
key_to_path(const char *key, size_t klen) {
	char *path, *p;
	const char *k;

	path = malloc(klen);

	for (k = key, p = path; k < key+klen; ++p) {
		printf("%x ", *k);
		if (strncmp(k, &(sep[0]), seplen) == 0) {
			*p = '/';
			k += seplen;
		} else {
			*p = *k;
			k++;
		}
	}
	printf("\n");
	fflush(stdout);

	*p = '\0';
	printf("path: %s\n", path);
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

