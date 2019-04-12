#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int fs_follow = 'P';
struct histnode *fs_hist;

static struct histnode *
popnode(struct histnode **hp)
{
	struct histnode *op;

	op  = *hp;
	*hp = op->next;

	return op;
}

int
open_dir(FS_DIR *dir, const char *path)
{
	struct stat st;
	struct histnode *hp;

	dir->dir  = (char *)path;
	dir->dlen = strlen(dir->dir);

	if (!(dir->dirp = opendir(dir->dir)))
		return FS_ERR;

	if (fstat(dirfd(dir->dirp), &st) < 0) {
		closedir(dir->dirp);
		return FS_ERR;
	}

	for (hp = fs_hist; hp; hp = hp->next) {
		if (st.st_dev == hp->dev && st.st_ino == hp->ino) {
			closedir(dir->dirp);
			return FS_CONT;
		}
	}

	hp       = emalloc(sizeof(*hp));
	hp->dev  = st.st_dev;
	hp->ino  = st.st_ino;
	hp->next = fs_hist;
	fs_hist  = hp;

	return FS_OK;
}

int
read_dir(FS_DIR *dir)
{
	struct dirent *entry;
	int (*statf)(const char *, struct stat *);

	statf = FS_FOLLOW(1) ? stat : lstat;

	if ((entry = readdir(dir->dirp))) {
		dir->name = entry->d_name;
		dir->nlen = strlen(dir->name);

		snprintf(dir->path, sizeof(dir->path),
		         "%s/%s", dir->dir, dir->name);

		if (statf(dir->path, &dir->info) < 0)
			return FS_ERR;
	} else {
		return FS_OK;
	}

	return FS_EXEC;
}

void
close_dir(FS_DIR *dir)
{
	closedir(dir->dirp);
	while (fs_hist)
		free(popnode(&fs_hist));
}
