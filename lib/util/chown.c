#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "util.h"

int chown_hflag;

static int
afile(const char *s, uid_t uid, gid_t gid, int depth, struct stat *st)
{
	int (*chownf)(const char *, uid_t, gid_t);

	if (!S_ISLNK(st->st_mode))
		chownf = chown;
	else
		chownf = lchown;

	if (chownf(s, (uid == (uid_t)-1) ? st->st_uid : uid, gid) < 0) {
		warn("(l)chown %s", s);
		return 1;
	}

	return 0;
}

int
chownfile(const char *s, uid_t uid, gid_t gid, int depth)
{
	struct stat st;

	if ((FS_FOLLOW(depth) ||
	    (chown_hflag & !depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s", s);
		return 1;
	}

	return afile(s, uid, gid, depth, &st);
}

int
chowndir(const char *s, uid_t uid, gid_t gid, int depth)
{
	FS_DIR dir;
	int rd, rval;

	rval = 0;

	switch (open_dir(&dir, s)) {
	case FS_ERR:
		if (errno != ENOTDIR) {
			warn("open_dir %s", s);
			return 1;
		}
		rval = chownfile(s, uid, gid, depth);
		/* fallthrough */
	case FS_CONT:
		return rval;
	}

	depth++;
	while ((rd = read_dir(&dir)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		rval |= afile(s, uid, gid, depth, &dir.info);

		if (S_ISDIR(dir.info.st_mode))
			rval |= chowndir(s, uid, gid, depth);
	}
	depth--;

	close_dir(&dir);

	if (rd == FS_ERR) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	return rval;
}
