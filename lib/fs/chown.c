/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "fs.h"

int
chown_file(const char *s, uid_t uid, gid_t gid, int depth)
{
	int (*chownf)(const char *, uid_t, gid_t);
	struct stat st;

	if ((FS_FOLLOW(depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s:", s);
		return 1;
	}

	if (!S_ISLNK(st.st_mode))
		chownf = chown;
	else
		chownf = lchown;

	if (chownf(s, (uid == (uid_t)-1) ? st.st_uid : uid, gid) < 0) {
		warn("(l)chown %s:", s);
		return 1;
	}

	return 0;
}

int
chown_folder(const char *s, uid_t uid, gid_t gid, int depth)
{
	FS_DIR dir;
	int rval = 0;

	if (open_dir(&dir, s) < 0) {
		rval = (errno == ENOTDIR);

		if (rval)
			rval = chown_file(s, uid, gid, depth);
		else
			warn("open_dir %s:", s);

		return rval;
	}

	while (read_dir(&dir, depth) != EOF) {
		if (ISDOT(dir.name))
			continue;

		rval |= chown_file(s, uid, gid, depth);
		if (S_ISDIR(dir.info.st_mode))
			rval |= chown_folder(s, uid, gid, depth + 1);
	}

	return rval;
}
