
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

/*
 * returns true if base_key is a sublevel of key
 */
const char
key_is_base(const char *base_key, size_t base_key_len,
            const char *key, size_t klen);

/*
 * append the seperator to a given key
 */
char *
key_append_sep(char *key, size_t *klen);

/*
 * compare tow keys
 */
int
keycmp(const char *key1, size_t klen1,
       const char *key2, size_t klen2);
