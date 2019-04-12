#include <sys/stat.h>

#include <err.h>
#include <errno.h>

#include "util.h"

int
chmodfile(const char *s, mode_t mode, int depth)
{
	struct stat st;

	if ((FS_FOLLOW(depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s", s);
		return 1;
	}

	if (chmod(s, mode ? mode : st.st_mode) < 0) {
		warn("chmod %s", s);
		return 1;
	}

	return 0;
}

int
chmoddir(const char *s, mode_t mode, int depth)
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
		rval = chmodfile(s, mode, depth);
		/* fallthrough */
	case FS_CONT:
		return rval;
	}

	depth++;
	while ((rd = read_dir(&dir)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		if (chmod(s, mode ? mode : dir.info.st_mode) < 0) {
			warn("chmod %s", s);
			rval = 1;
		}

		if (S_ISDIR(dir.info.st_mode))
			rval |= chmoddir(dir.path, mode, depth);
	}
	depth--;

	close_dir(&dir);

	if (rd == FS_ERR) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	return rval;
}
