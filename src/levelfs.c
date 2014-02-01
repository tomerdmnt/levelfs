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
static int levelfs_getattr(const char *, struct stat *);
static int levelfs_readdir(const char *, void *, fuse_fill_dir_t,
  	                   off_t, struct fuse_file_info *);
static int levelfs_open(const char *, struct fuse_file_info *);
static int levelfs_read(const char *, char *, size_t,
	                off_t, struct fuse_file_info *);

static struct fuse_operations levelfs_oper = {
	.init     = levelfs_init,
	.getattr  = levelfs_getattr,
	.readdir  = levelfs_readdir,
	.open     = levelfs_open,
	.read     = levelfs_read,
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

static int
levelfs_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	levelfs_iter_t *it;
	const char *key, *val;
	char *base_key;
	size_t base_key_len, klen, vlen;
	ctx_t *ctx = fuse_get_context()->private_data;

	memset(stbuf, sizeof(struct stat), 0);

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	base_key = path_to_key(path, strlen(path), &base_key_len);
	it = levelfs_iter_seek(ctx->db, base_key, base_key_len);

	key = levelfs_iter_next(it, &klen);

	if (!key) {
		res = -ENOENT;
	} else if (strncmp(base_key, key, klen) == 0) {
		val = levelfs_iter_value(it, &vlen);
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = vlen;
	} else {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
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
	char *base_key, *item_path, *item, *prev_item, *p;
	size_t base_key_len, klen;
	ctx_t *ctx = fuse_get_context()->private_data;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	base_key = path_to_key(path, strlen(path), &base_key_len);
	prev_item = malloc(1);
	prev_item[0] = '\0';

	it = levelfs_iter_seek(ctx->db, base_key, base_key_len);
	while ((key = levelfs_iter_next(it, &klen)) != NULL) {
		item_path = key_to_path(key, klen);
		if (strlen(item_path) < strlen(path))
			break;
		//TODO: new path function
		p = item_path + strlen(path);
		if (*p == '/') p++;
		item = strsep(&p, "/");

		if (strcmp(prev_item, item) == 0)
			continue;

		filler(buf, item, NULL, 0);

		free(prev_item);
		prev_item = malloc(strlen(item)+1);
		strcpy(prev_item, item);
	}

	free(base_key);
	levelfs_iter_close(it);
	free(prev_item);

	return 0;
}

static int
levelfs_open(const char *path, struct fuse_file_info *fi)
{
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

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

	key = path_to_key(path, strlen(path), &keylen);
	val = levelfs_db_get(ctx->db, key, keylen, &vallen, &err);
	free(key);

	if (err) {
		fprintf(stderr, "leveldb get error: %s\n", err);
		free(err);
		return 0;
	}

	if (vallen - offset < size) 
		size = vallen - offset;
	memcpy(buf, val + offset, size);


	return size;
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

