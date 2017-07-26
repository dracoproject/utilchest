/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-f] [-Rr] file ...";

static int
rm_file(const char *f, int silent, int depth)
{
	int rval = 0;
	struct stat st;

	if (lstat(f, &st) < 0) {
		if (!silent || errno != ENOENT)
			warn("lstat %s", f);

		return (!silent);
	}

	if (S_ISDIR(st.st_mode))
		rval = rmdir(f);
	else
		rval = unlink(f);

	if (rval < 0)
		warn("rm_file %s", f);

	return (rval < 0);
}

static int
rm_folder(const char *f, int silent, int depth)
{
	int rval = 0;
	FS_DIR dir;

	if (open_dir(&dir, f) < 0) {
		rval = (errno == ENOTDIR);

		if (rval)
			rval = rm_file(f, depth, silent);
		else
			warn("open_dir %s", f);

		return rval;
	}

	while (read_dir(&dir, depth) != EOF) {
		if (ISDOT(dir.name))
			continue;

		if (S_ISDIR(dir.info.st_mode))
			rval |= rm_folder(dir.path, silent, depth);
		else
			rval |= rm_file(dir.path, silent, depth);
	}

	if (rmdir(f) < 0) {
		warn("rmdir %s", f);
		rval = 1;
	}

	return rval;
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
		wrong(usage);
	} ARGEND

	if (!argc && !silent)
		wrong(usage);

	for (; *argv; argv++) {
		if (ISDOT(*argv))
			continue;

		rval |= rm(*argv, silent, 0);
	}

	return rval;
}
