/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

#define LN_F 0x1 /* force */
#define LN_L 0x2 /* follow symlink */
#define LN_S 0x4 /* create symlink */

SET_USAGE = "%s [-f] [-L|-P|-s] source [target]\n"
            "%s [-f] [-L|-P|-s] source ... dir";

static int
linkit(const char *src, const char *dest, int opts) {
	int rval = 0;

	if (opts & LN_F)
		unlink(dest);

	if (opts & LN_S) {
		if (symlink(src, dest))
			rval = pwarn("symlink %s -> %s:", src, dest);
	} else {
		if (linkat(AT_FDCWD, src, AT_FDCWD, dest,
		   (opts & LN_L) ? AT_SYMLINK_FOLLOW : 0))
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
		opts |= LN_F;
		break;
	case 's':
		opts |= LN_S;
		break;
	case 'L':
		opts |= LN_L;
		break;
	case 'P':
		opts |= ~LN_L;
		break;
	default:
		wrong(usage);
	} ARGEND

	switch (argc) {
	case 0:
		wrong(usage);
	case 1:
		exit(linkit(argv[0], ".", opts));
	case 2:
		exit(linkit(argv[0], argv[1], opts));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st))
		perr(1, "stat %s:", sourcedir);

	if (!S_ISDIR(st.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argv++)
		rval |= linkit(*argv, sourcedir, opts);

	return 0;
}
