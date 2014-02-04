
#include <stddef.h>

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
