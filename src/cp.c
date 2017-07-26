/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <stdlib.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-fp] [-R [-H|-L|-P]] source target\n"
            "%s [-fp] [-R [-H|-L|-P]] source ... dir";

int
main(int argc, char *argv[])
{
	const char *sourcedir;
	int (*cp)(const char *, const char *, int, int) = copy_file;
	int rval = 0, opts = 0;
	struct stat st;

	setprogname(argv[0]);

	ARGBEGIN {
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
		fs_follow = ARGC();
		break;
	default:
		wrong(usage);
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		wrong(usage);
	case 2:
		exit(cp(argv[0], pcat(argv[0], argv[1], 0), 0, opts));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argc--, argv++)
		rval |= cp(*argv, pcat(*argv, sourcedir, 1), 0, opts);

	return rval;
}
