
/*
 * in memory store for new empty directories
 * empty directories cannot persist between mounts
 * since they are not saved to the database
 */

/*
 * parent directory of new empty directory
 */
typedef struct parent_t {
	char            *path;
	struct parent_t *next;
	struct parent_t *prev;
	struct entry_t  *children;
} parent_t;

/*
 * new empty directory
 */
typedef struct entry_t {
	char           *name;
	struct entry_t *next;
	struct entry_t *prev;
} entry_t;

/*
 * add directory to the store
 */
void
newdirs_add(const char *path);

/*
 * remove directory from the store
 */
void
newdirs_remove(const char *path);

/*
 * returns true if directory exists
 */
char
newdirs_exists(const char *path);

/*
 * list empty new directories under path
 */
parent_t *
newdirs_list(const char *path);

