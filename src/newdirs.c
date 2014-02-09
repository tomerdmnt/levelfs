
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "newdirs.h"

enum {
	NEWDIRS_SIZE = 1024,
	MULTIPLIER   = 31,
};

typedef struct entry_t {
	char           *path;
	struct entry_t *next;
	struct entry_t *prev;
} entry_t;

entry_t *newdirs[NEWDIRS_SIZE];

uint64_t
hash(const char *path) {
	uint64_t h;
	unsigned char *p;

	for (p = (unsigned char *)path; p != '\0'; p++)
		h = MULTIPLIER * h + *p; 
	return h % NEWDIRS_SIZE;
}

entry_t *
lookup(const char *path, uint32_t h, char create) {
	entry_t *e;
	for (e = newdirs[h]; e != NULL; e = e->next) {
		if (strcmp(e->path, path) == 0)
			return e;
	}
	if (create) {
		e = malloc(sizeof(entry_t));
		e->path = strdup(path);
		e->prev = NULL;
		e->next = newdirs[h];
		e->next->prev = e;
		newdirs[h] = e;
		return e;
	}
	return NULL;
}

void
newdirs_add(const char *path) {
	uint64_t h;

	h = hash(path);
	lookup(path, h, 1);
}

void
newdirs_remove(const char *path) {
	entry_t *e;
	uint64_t h;

	h = hash(path);
	e = lookup(path, h, 0);
	if (e) {
		newdirs[h] = e->next;
		e->next->prev = NULL;
		free(e->path);
		free(e);
	}
}

char
newdirs_exists(const char *path) {
	entry_t *e;
	uint64_t h;

	h = hash(path);
	e = lookup(path, h, 0);
	return (e != NULL);
}


