
#include <stddef.h>

/*
 * compare string to sublevel seperator
 */
int
sepcmp(const char *str, size_t len);

/*
 * returns a db key representaiton of a path
 */
char *
path_to_key(const char *path, size_t *klen, char appendsep);

/*
 * returns a null terminated path representation of a db key
 */
char *
key_to_path(const char *key, size_t klen);

/*
 * returns diff between paths
 */
const char *
path_diff(const char *base_path, const char *path);

/*
 * dirname version that allocates a new buffer
 */
char *
dirname(const char *path);

/*
 * basename version that allocates a new buffer
 */
char *
basename(const char *path);
