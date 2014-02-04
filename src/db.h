
#include "../deps/leveldb/include/leveldb/c.h"

typedef struct {
	leveldb_t         *db;
	leveldb_options_t *opts;
} levelfs_db_t;

typedef struct {
	leveldb_iterator_t    *it;
	leveldb_readoptions_t *opts;
	char                  *base_key;
	size_t                base_key_len;
	int                   first;
} levelfs_iter_t;

/* 
 * open database
 */
levelfs_db_t *
levelfs_db_open(const char *path, char **errptr);

/*
 * close database
 */
void
levelfs_db_close(levelfs_db_t *db);

/*
 * db get
 */
const char *
levelfs_db_get(levelfs_db_t *db, const char *key, size_t klen,
               size_t *vlen, char **errptr);

/*
 * db put
 */
void
levelfs_db_put(levelfs_db_t *db, const char *key, size_t klen,
               const char *val, size_t vlen, char **errptr);

/*
 * create iterator for keys which begines with key
 */
levelfs_iter_t *
levelfs_iter_seek(levelfs_db_t *db, const char *base_key, size_t base_key_len);

/*
 * advances the iterator and returns the key
 */
const char *
levelfs_iter_next(levelfs_iter_t *it, size_t *klen);

/*
 * get the value of current item
 */
const char *
levelfs_iter_value(levelfs_iter_t *it, size_t *vlen);

/*
 * close iterator
 */
void
levelfs_iter_close(levelfs_iter_t *it); 

