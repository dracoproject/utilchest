#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static int fflag;

static int
afile(const char *f, int depth, struct stat *st)
{
	int (*rm)(const char *);

	if (S_ISDIR(st->st_mode))
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
delfile(const char *f, int depth)
{
	struct stat st;

	if (lstat(f, &st) < 0) {
		if (!fflag && errno != ENOENT)
			warn("lstat %s", f);
		return (!fflag);
	}

	return afile(f, depth, &st);
}

static int
deldir(const char *f, int depth)
{
	int rd, rval;
	FS_DIR dir;

	rval = 0;

	switch (open_dir(&dir, f)) {
	case FS_ERR:
		if (errno != ENOTDIR) {
			warn("open_dir %s", f);
			return 1;
		}
		rval = delfile(f, depth);
	case FS_CONT:
		return rval;
	}

	depth++;
	while ((rd = read_dir(&dir)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		if (S_ISDIR(dir.info.st_mode))
			rval |= deldir(dir.path, depth);
		else
			rval |= afile(dir.path, depth, &dir.info);
	}
	depth--;

	close_dir(&dir);

	if (rd == FS_ERR) {
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
	int (*rm)(const char *, int), rval;

	rm   = delfile;
	rval = 0;
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
