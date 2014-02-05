/*
 * FUSE file system for leveldb
 */
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

#include "path.h"
#include "db.h"

static void *levelfs_init(struct fuse_conn_info *);
static void levelfs_destroy(void *);
static int levelfs_getattr(const char *, struct stat *);
static int levelfs_readdir(const char *, void *, fuse_fill_dir_t,
  	                   off_t, struct fuse_file_info *);
static int levelfs_read(const char *, char *, size_t,
	                off_t, struct fuse_file_info *);
static int levelfs_write(const char *, const char *, size_t,
                         off_t, struct fuse_file_info *);
static int levelfs_mknod(const char *, mode_t, dev_t);
static int levelfs_truncate(const char *, off_t);
static int levelfs_open(const char *, struct fuse_file_info *);
static int levelfs_flush(const char *, struct fuse_file_info *);
static int levelfs_release(const char *, struct fuse_file_info *);
static int levelfs_chmod(const char *, mode_t);
static int levelfs_chown(const char *, uid_t, gid_t);
static int levelfs_utime(const char *, struct utimbuf *);

static struct fuse_operations levelfs_oper = {
	.init     = levelfs_init,
	.destroy  = levelfs_destroy,
	.getattr  = levelfs_getattr,
	.readdir  = levelfs_readdir,
	.read     = levelfs_read,
	.write    = levelfs_write,
	.mknod    = levelfs_mknod,
	.truncate = levelfs_truncate,
	.open     = levelfs_open,
	.flush    = levelfs_flush,
	.release  = levelfs_release,
	.chmod    = levelfs_chmod,
	.chown    = levelfs_chown,
	.utime    = levelfs_utime,
};

typedef struct {
	char        *db_path;
} conf_t;

static conf_t conf;

typedef struct {
	levelfs_db_t *db;
} ctx_t;

/*
 * ctx can be retreived using fuse_get_context()->private_data
 */
static void *
levelfs_init(struct fuse_conn_info *conn) {
	ctx_t *ctx;
	char *err = NULL;

	ctx = malloc(sizeof(ctx_t));
	ctx->db = levelfs_db_open(conf.db_path, &err);
	if (err) {
		fprintf(stderr, "error opening db: %s", err);
		exit(1);
	}

	return ctx;
}

static void
levelfs_destroy(void *ctx) {
	levelfs_db_close(((ctx_t *)ctx)->db);
}

static int
levelfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	levelfs_iter_t *it;
	const char *key, *val;
	char *base_key;
	size_t base_key_len, klen, vlen;
	struct fuse_context *fuse_ctx = fuse_get_context();
	ctx_t *ctx = fuse_ctx->private_data;

	memset(stbuf, sizeof(struct stat), 0);
	stbuf->st_uid = fuse_ctx->uid;
	stbuf->st_gid = fuse_ctx->gid;

	/* root directory */
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	base_key = path_to_key(path, &base_key_len);
	it = levelfs_iter_seek(ctx->db, base_key, base_key_len);
	key = levelfs_iter_next(it, &klen);
	if (!key) {
		res = -ENOENT;
	} else if (klen == base_key_len) {
		/* exact match = file */
		stbuf->st_mode = S_IFREG | 0666;
		stbuf->st_nlink = 1;
		val = levelfs_iter_value(it, &vlen);
		stbuf->st_size = vlen;
	} else if (path[strlen(path)-1] == '/') {
		/* partial match = directory */
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		res = -ENOENT;
	}

	free(base_key);
	levelfs_iter_close(it);

	return res;
}

static int 
levelfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi)
{
	levelfs_iter_t *it;
	const char *key;
	char *base_key, *item_path, *item, *prev_item, *pdiff;
	size_t base_key_len, klen;
	ctx_t *ctx = fuse_get_context()->private_data;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	base_key = path_to_key(path, &base_key_len);
	item_path = malloc(1);
	prev_item = malloc(1);
	prev_item[0] = '\0';

	it = levelfs_iter_seek(ctx->db, base_key, base_key_len);

	while ((key = levelfs_iter_next(it, &klen)) != NULL) {
		free(item_path);
		item_path = key_to_path(key, klen);
		pdiff = (char *)path_diff(path, item_path);
		if (!pdiff) continue;
		item = strsep(&pdiff, "/");

		if (strcmp(prev_item, item) != 0) {
			/* add item to directory */
			filler(buf, item, NULL, 0);

			free(prev_item);
			prev_item = malloc(strlen(item)+1);
			strcpy(prev_item, item);
		}
	}

	levelfs_iter_close(it);
	free(base_key);
	free(item_path);
	free(prev_item);

	return 0;
}

static int 
levelfs_read(const char *path, char *buf, size_t size, off_t offset,
           struct fuse_file_info *fi)
{
	char *key;
	const char *val;
	size_t keylen, vallen;
	char *err = NULL;
	ctx_t *ctx = fuse_get_context()->private_data;

	key = path_to_key(path, &keylen);
	val = levelfs_db_get(ctx->db, key, keylen, &vallen, &err);
	free(key);

	if (err) {
		fprintf(stderr, "leveldb get error: %s\n", err);
		free(err);
		return 0;
	}

	if (vallen - offset < size) 
		size = vallen - offset;
	memcpy(buf, val+offset, size);

	return size;
}

static int
levelfs_write(const char *path, const char *buf, size_t bufsize,
              off_t offset, struct fuse_file_info *fi) {
	int res;
	char *key, *new_val;
	const char *val;
	size_t klen, vlen, new_val_len;
	char *err = NULL;
	ctx_t *ctx = fuse_get_context()->private_data;

	res = bufsize;
	new_val = NULL;

	key = path_to_key(path, &klen);
	val = levelfs_db_get(ctx->db, key, klen, &vlen, &err);
	if (err) {
		res = -ENOENT;
		goto clean;
	}
	if (offset + bufsize > vlen) {
		/* allocate a bigger buffer */
		new_val_len = offset + bufsize;
		new_val = malloc(new_val_len);
		memcpy(new_val, val, offset);
	} else {
		new_val_len = vlen;
		new_val = malloc(new_val_len);
		memcpy(new_val, val, new_val_len);
	}
	memcpy(new_val+offset, buf, bufsize);
	levelfs_db_put(ctx->db, key, klen, new_val, new_val_len, &err);
	if (err) {
		fprintf(stderr, "leveldb put error: %s\n", err);
		// TODO: change errno
		res = -ENOENT;
	}

clean:
	free(key);
	if (new_val) free(new_val);
	if (err) leveldb_free(err);

	return res;
}

static int
levelfs_mknod(const char *path, mode_t mode, dev_t dev) {
	char *key;
	size_t klen;
	char *err = NULL;
	ctx_t *ctx = fuse_get_context()->private_data;

	key = path_to_key(path, &klen);
	levelfs_db_put(ctx->db, key, klen, NULL, 0, &err);
	free(key);
	if (err) {
		fprintf(stderr, "leveldb put error: %s\n", err);
		// TODO: change errno
		return -ENOENT;
	}

	return 0;
}

static int
levelfs_truncate(const char *path, off_t size) {
	const char *val;
	char *key;
	size_t vlen, klen;
	char *err = NULL;
	ctx_t *ctx = fuse_get_context()->private_data;

	key = path_to_key(path, &klen);
	val = levelfs_db_get(ctx->db, key, klen, &vlen, &err);
	if (err) {
		free(key);
		free(err);
		return -ENOENT;
	}
	if (vlen > size) {
		levelfs_db_put(ctx->db, key, klen, val, size, &err);
		if (err) {
			fprintf(stderr, "leveldb put: %s\n", err);
			free(err);
			free(key);
			// TODO: change errno
			return -ENOENT;
		}
	}
	return 0;
}

static int
levelfs_open(const char *path, struct fuse_file_info *fi)
{
	return 0;
}

static int
levelfs_flush(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int
levelfs_release(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int levelfs_chmod(const char *path, mode_t mode) {
	return 0;
}

static int
levelfs_chown(const char *path, uid_t uid, gid_t gid) {
	return 0;
}

static int
levelfs_utime(const char *path, struct utimbuf *ubuf) {
	return 0;
}

static void
usage(char *proc) {
	fprintf(stderr,
	    "usage: %s dbpath mountpoint [options]\n"
	    "\n"
	    "general options:\n"
	    "    -o opt,[opt...]        mount options\n"
	    "    -h   --help            print help\n"
	    "    -V   --version         print version\n"
	    "\n"
	    "FUSE options:\n"
	    "    -d   -o debug          enable debug output (implies -f)\n"
	    "    -f                     foreground operation\n"
	    "    -s                     disable multi-threaded operation\n"
	    , proc);
}

enum {
     KEY_HELP,
     KEY_VERSION,
};

static struct fuse_opt opts[] = {
	FUSE_OPT_KEY("-V",            KEY_VERSION),
	FUSE_OPT_KEY("--version",     KEY_VERSION),
	FUSE_OPT_KEY("-h",            KEY_HELP),
	FUSE_OPT_KEY("--help",        KEY_HELP),
	FUSE_OPT_END
};

static int
opt_parse(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	switch(key) {
		case FUSE_OPT_KEY_NONOPT:
			if (!conf.db_path) {
				conf.db_path = realpath(arg, NULL);
				if (!conf.db_path) perror(arg);
				return 0;
			}
			return 1;
		case KEY_HELP:
			usage(outargs->argv[0]);
			exit(1);
		case KEY_VERSION:
			fprintf(stderr, "v0.1\n");
			exit(0);
	}
	return 1;
}

#ifndef NO_MAIN
int
main(int argc, char **argv)
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&conf, 0, sizeof(conf_t));

	fuse_opt_parse(&args, &conf, opts, opt_parse);

	return fuse_main(args.argc, args.argv, &levelfs_oper, NULL);
}
#endif

