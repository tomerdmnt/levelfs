/*
 * FUSE file system for leveldb
 */
#define FUSE_USE_VERSION 26
#define LEVELFS_VERSION "0.0.2"

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

#include "path.h"
#include "db.h"
#include "newdirs.h"

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
static int levelfs_unlink(const char *);
static int levelfs_truncate(const char *, off_t);
static int levelfs_mkdir(const char *, mode_t);
static int levelfs_rmdir(const char *);
static int levelfs_open(const char *, struct fuse_file_info *);
static int levelfs_flush(const char *, struct fuse_file_info *);
static int levelfs_release(const char *, struct fuse_file_info *);
static int levelfs_chmod(const char *, mode_t);
static int levelfs_chown(const char *, uid_t, gid_t);
static int levelfs_utime(const char *, struct utimbuf *);
static int levelfs_setxattr(const char *, const char *, const char *,
                            size_t, int, uint32_t);
static int levelfs_getxattr(const char *, const char *, char *,
                            size_t, uint32_t);
static int levelfs_listxattr(const char *, char *, size_t);
static int levelfs_removexattr(const char *, const char *);

static struct fuse_operations levelfs_oper = {
	.init        = levelfs_init,
	.destroy     = levelfs_destroy,
	.getattr     = levelfs_getattr,
	.readdir     = levelfs_readdir,
	.read        = levelfs_read,
	.write       = levelfs_write,
	.mknod       = levelfs_mknod,
	.unlink      = levelfs_unlink,
	.truncate    = levelfs_truncate,
	.mkdir       = levelfs_mkdir,
	.rmdir       = levelfs_rmdir,
	.open        = levelfs_open,
	.flush       = levelfs_flush,
	.release     = levelfs_release,
	.chmod       = levelfs_chmod,
	.chown       = levelfs_chown,
	.utime       = levelfs_utime,
	.setxattr    = levelfs_setxattr,
	.getxattr    = levelfs_getxattr,
	.listxattr   = levelfs_listxattr,
	.removexattr = levelfs_removexattr,
};

/*
 * conf_t used by fuse opts parser
 */
typedef struct {
	char        *db_path;
} conf_t;

static conf_t conf;

/*
 * fuse context private data
 */
typedef struct {
	db_t *db;
} ctx_t;

#define CTX_DB ((ctx_t *)(fuse_get_context()->private_data))->db

/*
 * ctx can be retreived using CTX_DB
 */
static void *
levelfs_init(struct fuse_conn_info *conn) {
	ctx_t *ctx;
	char *err = NULL;

	ctx = malloc(sizeof(ctx_t));
	ctx->db = db_open(conf.db_path, &err);
	if (err) {
		fprintf(stderr, "error opening db: %s", err);
		exit(1);
	}

	return ctx;
}

/*
 * unmount
 */
static void
levelfs_destroy(void *ctx) {
	db_close(((ctx_t *)ctx)->db);
}

/*
 * determine directory entry type, i.e. dir/file
 */
static int
levelfs_getattr(const char *path, struct stat *stbuf)
{
	int res;
	db_iter_t *it;
	const char *key, *val;
	char *base_key;
	size_t base_key_len, klen, vlen;
	struct fuse_context *fuse_ctx = fuse_get_context();

	memset(stbuf, sizeof(struct stat), 0);
	stbuf->st_uid = fuse_ctx->uid;
	stbuf->st_gid = fuse_ctx->gid;

	/* root directory */
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}
	/* empty dir */
	if (newdirs_exists(path)) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	res = -ENOENT;
	base_key = path_to_key(path, &base_key_len, 0);
	it = db_iter_seek(CTX_DB, base_key, base_key_len);

	while ((key = db_iter_next(it, &klen)) != NULL) {
		if (base_key_len == klen) {
			/* exact match = file */
			stbuf->st_mode = S_IFREG | 0666;
			stbuf->st_nlink = 1;
			val = db_iter_value(it, &vlen);
			stbuf->st_size = vlen;
			res = 0;
			break;
		} else if (sepcmp(key+base_key_len, klen - base_key_len) == 0) {
			/* sublevel = directory */
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			res = 0;
			break;
		}
	}

	db_iter_close(it);
	free(base_key);

	return res;
}

/*
 * get all entries in directory
 */
static int 
levelfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                off_t offset, struct fuse_file_info *fi)
{
	db_iter_t *it;
	const char *key;
	char *base_key, *item_path, *item, *prev_item, *pdiff;
	size_t base_key_len, klen;
	parent_t *p;
	entry_t *e;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	/* new directories */
	e = NULL;
	p = newdirs_list(path);
	if (p) e = p->children;
	for (; e != NULL; e = e->next)
		filler(buf, e->name, NULL, 0);

	item_path = malloc(1);
	prev_item = malloc(1);
	prev_item[0] = '\0';
	base_key = path_to_key(path, &base_key_len, 1);

	it = db_iter_seek(CTX_DB, base_key, base_key_len);
	while ((key = db_iter_next(it, &klen)) != NULL) {
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
	db_iter_close(it);

	free(base_key);
	free(item_path);
	free(prev_item);
	return 0;
}

/*
 * read file
 */
static int 
levelfs_read(const char *path, char *buf, size_t size, off_t offset,
           struct fuse_file_info *fi)
{
	char *key;
	const char *val;
	size_t keylen, vallen;
	char *err = NULL;

	key = path_to_key(path, &keylen, 0);
	val = db_get(CTX_DB, key, keylen, &vallen, &err);
	free(key);

	if (err) {
		fprintf(stderr, "leveldb get error: %s\n", err);
		free(err);
		return 0;
	}

	if (vallen < offset)
		return 0;
	if (vallen - offset < size) 
		size = vallen - offset;
	memcpy(buf, val+offset, size);

	return size;
}

/*
 * write file
 */
static int
levelfs_write(const char *path, const char *buf, size_t bufsize,
              off_t offset, struct fuse_file_info *fi) {
	int res;
	char *key, *new_val;
	const char *val;
	size_t klen, vlen, new_val_len;
	char *err = NULL;

	res = 0;
	new_val = NULL;

	key = path_to_key(path, &klen, 0);
	val = db_get(CTX_DB, key, klen, &vlen, &err);
	if (err) {
		res = -ENOENT;
		goto error;
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
	db_put(CTX_DB, key, klen, new_val, new_val_len, &err);
	if (err) {
		fprintf(stderr, "leveldb put error: %s\n", err);
		// TODO: change errno
		res = -ENOENT;
	}
	res = bufsize;

error:
	free(key);
	free(new_val);
	leveldb_free(err);

	return res;
}

/*
 * touch file
 */
static int
levelfs_mknod(const char *path, mode_t mode, dev_t dev) {
	char *key, *parent;
	size_t klen;
	char *err = NULL;

	key = path_to_key(path, &klen, 0);
	db_put(CTX_DB, key, klen, NULL, 0, &err);
	free(key);
	if (err) {
		fprintf(stderr, "leveldb put error: %s\n", err);
		free(err);
		// TODO: change errno
		return -ENOENT;
	}
	parent = dirname(path);
	newdirs_remove(parent);
	free(parent);

	return 0;
}

/*
 * rm file
 */
static int
levelfs_unlink(const char *path) {
	char *key;
	size_t klen;
	char *err = NULL;

	key = path_to_key(path, &klen, 0);
	db_del(CTX_DB, key, klen, &err);
	if (err) {
		fprintf(stderr, "leveldb del error: %s", err);
		return -ENOENT;
	}
	return 0;
}

/*
 * truncate file from off
 */
static int
levelfs_truncate(const char *path, off_t offset) {
	int res;
	const char *val;
	char *key;
	size_t vlen, klen;
	char *err = NULL;

	res = 0;
	key = path_to_key(path, &klen, 0);
	val = db_get(CTX_DB, key, klen, &vlen, &err);
	if (err) {
		res = -ENOENT;
		goto error;
	}
	if (vlen > offset) {
		db_put(CTX_DB, key, klen, val, offset, &err);
		if (err) {
			fprintf(stderr, "leveldb put: %s\n", err);
			// TODO: change errno
			res = -ENOENT;
			goto error;
		}
	}

error:
	free(key);
	free(err);
	return res;
}

/*
 * create new directory
 */
static int
levelfs_mkdir(const char *path, mode_t mode) {
	int res;
	char *base_key;
	const char *key;
	size_t base_key_len, klen;
	db_iter_t *it;

	res = 0;
	base_key = path_to_key(path, &base_key_len, 0);
	if (newdirs_exists(path)) {
		res = -EEXIST;
		goto error;
	}
	it = db_iter_seek(CTX_DB, base_key, base_key_len);
	if ((key = db_iter_next(it, &klen)) != NULL) {
		if (base_key_len == klen ||
		    sepcmp(key+base_key_len, klen - base_key_len) == 0) {
			printf("mkdir: %s found in leveldb\n", path);
			res = -EEXIST;
			goto error;
		}
	}
	newdirs_add(path);

error:
	free(base_key);
	return res;
}

/*
 * remove empty directory
 */
static int
levelfs_rmdir(const char *path) {
	int res;
	char *base_key;
	const char *key;
	size_t base_key_len, klen;
	db_iter_t *it;

	res = 0;
	base_key = path_to_key(path, &base_key_len, 0);

	it = db_iter_seek(CTX_DB, base_key, base_key_len);
	if ((key = db_iter_next(it, &klen)) != NULL) {
		if (base_key_len == klen) {
			res = -ENOTDIR;
			goto error;
		} else if (sepcmp(key+base_key_len, klen - base_key_len) == 0) {
			res = -ENOTEMPTY;
			goto error;
		}
	}
	if (!newdirs_exists(path)) {
		res = -ENOENT;
		goto error;
	}
	if (newdirs_list(path) != NULL) {
		res = -ENOTEMPTY;
		goto error;
	}
	newdirs_remove(path);

error:
	free(base_key);
	return res;
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

static int
levelfs_chmod(const char *path, mode_t mode) {
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

static int
levelfs_setxattr(const char *path, const char *name,
                 const char *value, size_t size, int flags, uint32_t pos) {
	return 0;
}

static int
levelfs_getxattr(const char *path, const char *name,
                 char *value, size_t size, uint32_t pos) {
	return 0;
}

static int
levelfs_listxattr(const char *path, char *list, size_t size) {
	return 0;
}

static int
levelfs_removexattr(const char *path, const char *name) {
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

static char *unrealpath(const char *name) {
	if (mkdir(name, 0777) != 0) {
		if (errno != EEXIST) {
			perror("error creating directory");
			return NULL;
		}
	}
	return realpath(name, NULL);
}

static int
opt_parse(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	switch(key) {
		case FUSE_OPT_KEY_NONOPT:
			if (!conf.db_path) {
				conf.db_path = unrealpath(arg);
				if (!conf.db_path) perror(arg);
				return 0;
			}
			return 1;
		case KEY_HELP:
			usage(outargs->argv[0]);
			exit(1);
		case KEY_VERSION:
			fprintf(stderr, "v%s\n", LEVELFS_VERSION);
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

