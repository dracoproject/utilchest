#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

enum RET {
	EXE = 1,
	END = 0,
	ERR = -1
};

int fs_follow = 'P';

int
open_dir(FS_DIR *dir, const char *path)
{
	dir->dir=  (char *)path;
	dir->dlen= strlen(dir->dir);

	if (!(dir->dirp = opendir(dir->dir)))
		return -1;

	return 0;
}

int
read_dir(FS_DIR *dir, int rtime)
{
	int rval = EXE;
	int (*statf)(const char *, struct stat *);
	struct dirent *entry;

	if (FS_FOLLOW(rtime))
		statf = stat;
	else
		statf = lstat;

	if ((entry = readdir(dir->dirp))) {
		dir->name = entry->d_name;
		dir->nlen = strlen(dir->name);

		snprintf(dir->path, sizeof(dir->path),
		    "%s/%s", dir->dir, dir->name);

		if (statf(dir->path, &dir->info) < 0) {
			rval = ERR;
			goto clean;
		}
	} else {
		rval = END;
		goto clean;
	}

	goto done;
clean:
	closedir(dir->dirp);
done:
	return rval;
}
