#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static int fflag;

static int
delfile(const char *f, int depth)
{
	int (*rm)(const char *);
	struct stat st;

	if (lstat(f, &st) < 0) {
		if (!fflag && errno != ENOENT)
			warn("lstat %s", f);
		return (!fflag);
	}

	if (S_ISDIR(st.st_mode))
		rm = rmdir;
	else
		rm = unlink;

	if (rm(f) < 0) {
		warn("delfile %s", f);
		return 1;
	}

	return 0;
}

static int
deldir(const char *f, int depth)
{
	int rd, rval = 0;
	FS_DIR dir;

	if (open_dir(&dir, f) < 0) {
		if (!(rval = errno != ENOTDIR))
			rval = delfile(f, depth);
		else
			warn("open_dir %s", f);

		return rval;
	}

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		if (S_ISDIR(dir.info.st_mode))
			rval |= deldir(dir.path, depth);
		else
			rval |= delfile(dir.path, depth);
	}

	if (rd < 0) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	if (rmdir(f) < 0) {
		warn("deldir %s", f);
		return 1;
	}

	return rval;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-f] [-Rr] file ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int (*rm)(const char *, int) = delfile;
	int rval = 0;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		fflag = 1;
		break;
	case 'r':
	case 'R':
		rm = deldir;
		break;
	default:
		usage();
	} ARGEND

	if (!argc && !fflag)
		usage();

	for (; *argv; argv++) {
		if (ISDOT(*argv))
			continue;

		rval |= rm(*argv, 0);
	}

	return rval;
}
