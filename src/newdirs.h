
/*
 * in memory store for new empty directories
 * empty directories cannot persist between mounts
 * since they are not saved to the database
 */

void
newdirs_add(const char *path);

void
newdirs_del(const char *path);

const char *
newdirs_contains(const char *path);

