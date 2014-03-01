
#include <stddef.h>

/*
 * get the sublevel seperator character
 */
char
sublevel_seperator();

/*
 * returns a db key representaiton of a path
 */
char *
path_to_key(const char *path, size_t *klen);

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
 * append the seperator to a given key
 */
char *
key_append_sep(char *key, size_t *klen);

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
