/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

#define FORCE  0x1 /* force */
#define FOLLOW 0x2 /* follow symlink */
#define SYMLNK 0x4 /* create symlink */

SET_USAGE = "%s [-f] [-L|-P|-s] source [target]\n"
            "%s [-f] [-L|-P|-s] source ... dir";

static int
linkit(const char *src, const char *dest, int opts) {
	int rval = 0;

	if (opts & FORCE)
		unlink(dest);

	if (opts & SYMLNK) {
		if (symlink(src, dest))
			rval = pwarn("symlink %s -> %s:", src, dest);
	} else {
		if (linkat(AT_FDCWD, src, AT_FDCWD, dest,
		   (opts & FOLLOW) ? AT_SYMLINK_FOLLOW : 0))
			rval = pwarn("linkat %s:", src, dest);
	}


	return rval;
}

int
main(int argc, char *argv[]) {
	const char *sourcedir;
	int opts = 0, rval = 0;
	struct stat st;

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
		perr(1, "stat %s:", sourcedir);

	if (!S_ISDIR(st.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argv++)
		rval |= linkit(*argv, pcat(argv[0], sourcedir, 1), opts);

	return 0;
}
