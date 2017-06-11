/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "util.h"

int fst_follow = 'P';

int
fst_open(const char *p, FST_DIR *dir) {
	dir->dir= (char *)p;
	dir->dlen= strlen(dir->dir);

	if (!(dir->dirp = opendir(dir->dir)))
		return -1;

	return 0;
}

int
fst_read(FST_DIR *dir, int rtime) {
	char buf[PATH_MAX];
	int (*statf)(const char *, struct stat *);
	struct dirent *entry;

	if (FST_FOLLOW(rtime))
		statf = stat;
	else
		statf = lstat;

	if ((entry = readdir(dir->dirp))) {
		dir->name = entry->d_name;
		dir->nlen = strlen(dir->name);

		if ((dir->dlen + dir->nlen + 2) >= sizeof(buf)) {
			errno = ENAMETOOLONG;
			return -1;
		}

		dir->path = buf;
		dir->plen = snprintf(buf, sizeof(buf),
		            "%s/%s", dir->dir, dir->name);

		if (statf(dir->path, &dir->info) < 0)
			return -1;
	} else {
		closedir(dir->dirp);

		return -1;
	}

	return 0;
}
