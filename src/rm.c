/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static int
rm_file(const char *f, int silent, int depth)
{
	int (*rm)(const char *);
	struct stat st;

	if (lstat(f, &st) < 0) {
		if (errno != ENOENT)
			warn("lstat %s", f);
		return (!silent);
	}

	if (S_ISDIR(st.st_mode))
		rm = rmdir;
	else
		rm = unlink;

	if (rm(f) < 0) {
		warn("rm_file %s", f);
		return 1;
	}

	return 0;
}

static int
rm_folder(const char *f, int silent, int depth)
{
	int rd, rval = 0;
	FS_DIR dir;

	if (open_dir(&dir, f) < 0) {
		if (!(rval = errno != ENOTDIR))
			rval = rm_file(f, depth, silent);
		else
			warn("open_dir %s", f);

		return rval;
	}

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		if (S_ISDIR(dir.info.st_mode))
			rval |= rm_folder(dir.path, silent, depth);
		else
			rval |= rm_file(dir.path, silent, depth);
	}

	if (rd < 0) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	if (rmdir(f) < 0) {
		warn("rmdir %s", f);
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
	int (*rm)(const char *, int, int) = rm_file;
	int silent = 0, rval = 0;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		silent = 1;
		break;
	case 'r':
	case 'R':
		rm = rm_folder;
		break;
	default:
		usage();
	} ARGEND

	if (!argc && !silent)
		usage();

	for (; *argv; argv++) {
		if (ISDOT(*argv))
			continue;

		rval |= rm(*argv, silent, 0);
	}

	return rval;
}
