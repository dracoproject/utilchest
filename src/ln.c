/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

#define FORCE  0x1 /* force */
#define FOLLOW 0x2 /* follow symlink */
#define SYMLNK 0x4 /* create symlink */

SET_USAGE = "%s [-f] [-L|-P|-s] source [target]\n"
    "%s [-f] [-L|-P|-s] source [target]";

static int
linkit(const char *src, const char *dest, int opts)
{
	int flags = 0;

	if (opts & FORCE)
		unlink(dest);

	if (opts & FOLLOW)
		flags |= AT_SYMLINK_FOLLOW;

	if (opts & SYMLNK) {
		if (symlink(src, dest) < 0) {
			warn("symlink %s -> %s", src, dest);
			return 1;
		}
	} else {
		if (linkat(AT_FDCWD, src, AT_FDCWD, dest, flags) < 0) {
			warn("linkat %s -> %s", src, dest);
			return 1;
		}
	}


	return 0;
}

int
main(int argc, char *argv[])
{
	const char *sourcedir;
	int opts = 0, rval = 0;
	struct stat st;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		opts |= FORCE;
		break;
	case 's':
		opts |= SYMLNK;
		break;
	case 'L':
		opts |= FOLLOW;
		break;
	case 'P':
		opts |= ~FOLLOW;
		break;
	default:
		wrong(usage);
	} ARGEND

	switch (argc) {
	case 0:
		wrong(usage);
	case 1:
		exit(linkit(argv[0], pcat(argv[0], ".", 1), opts));
	case 2:
		exit(linkit(argv[0], pcat(argv[0], argv[1], 0), opts));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st))
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argv++)
		rval |= linkit(*argv, pcat(argv[0], sourcedir, 1), opts);

	return rval;
}
