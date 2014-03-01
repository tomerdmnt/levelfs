
#include "../deps/leveldb/include/leveldb/c.h"

typedef struct {
	leveldb_t         *db;
	leveldb_options_t *opts;
} db_t;

typedef struct {
	leveldb_iterator_t    *it;
	leveldb_readoptions_t *opts;
	char                  *base_key;
	size_t                base_key_len;
	int                   first;
} db_iter_t;

/* 
 * open database
 */
db_t *
db_open(const char *path, char **errptr);

/*
 * close database
 */
void
db_close(db_t *db);

/*
 * db get
 */
const char *
db_get(db_t *db, const char *key, size_t klen,
       size_t *vlen, char **errptr);

/*
 * db put
 */
void
db_put(db_t *db, const char *key, size_t klen,
       const char *val, size_t vlen, char **errptr);

/*
 * db delete
 */
void
db_del(db_t *db, const char *key,
       size_t klen, char **errptr);

/*
 * create iterator for keys which begines with key
 */
db_iter_t *
db_iter_seek(db_t *db, const char *base_key,
             size_t base_key_len);

/*
 * advances the iterator and returns the key
 */
const char *
db_iter_next(db_iter_t *it, size_t *klen);

/*
 * get the value of current item
 */
const char *
db_iter_value(db_iter_t *it, size_t *vlen);

/*
 * close iterator
 */
void
db_iter_close(db_iter_t *it);

