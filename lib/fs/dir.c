/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "util.h"

int fs_follow = 'P';

int
open_dir(const char *p, FS_DIR *dir)
{
	dir->dir= (char *)p;
	dir->dlen= strlen(dir->dir);
	dir->path= NULL;

	if (!(dir->dirp = opendir(dir->dir)))
		return -1;

	return 0;
}

int
read_dir(FS_DIR *dir, int rtime)
{
	int (*statf)(const char *, struct stat *);
	struct dirent *entry;

	if (FS_FOLLOW(rtime))
		statf = stat;
	else
		statf = lstat;

	if ((entry = readdir(dir->dirp))) {
		dir->name = entry->d_name;
		dir->nlen = strlen(dir->name);

		free(dir->path);
		if (!(dir->path = malloc(dir->dlen + dir->nlen + 2)))
			perr(1, "malloc:");

		dir->plen = sprintf(dir->path, "%s/%s", dir->dir, dir->name);

		if (statf(dir->path, &dir->info) < 0)
			return -1;
	} else {
		free(dir->path);
		dir->path = NULL;

		closedir(dir->dirp);

		return -1;
	}

	return 0;
}
