/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <stdlib.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-afp] [-R [-H|-L|-P]] source target\n"
            "%s [-afp] [-R [-H|-L|-P]] source ... dir";

int
main(int argc, char *argv[]) {
	const char *sourcedir;
	int (*cp)(const char *, const char *, int) = copy_file;
	int rval = 0, opts = 0;
	struct stat st;

	ARGBEGIN {
	case 'a':
		cp = copy_folder;
		tfh_follow = 'P';
		opts |= CP_PFLAG;
		break;
	case 'f':
		opts |= CP_FFLAG;
		break;
	case 'p':
		opts |= CP_PFLAG;
		break;
	case 'r':
	case 'R':
		cp = copy_folder;
		break;
	case 'H':
	case 'L':
	case 'P':
		tfh_follow = ARGC();
		break;
	default:
		wrong(usage);
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		wrong(usage);
	case 2:
		exit(cp(argv[0], argv[1], opts));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		perr(1, "stat %s:", sourcedir);

	if (!S_ISDIR(st.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argc--, argv++)
		rval |= cp(*argv, sourcedir, opts);

	return rval;
}
