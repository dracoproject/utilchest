/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-f] source target\n"
            "%s [-f] source ... target_dir";

static int
move(const char *src, const char *dest) {
	if (!rename(src, dest))
		return 0;

	if (errno == EXDEV)
		return (copy_folder(src, dest, CP_P));

	return (pwarn("rename %s/%s:", src, dest));
}

int
main(int argc, char *argv[]) {
	const char *sourcedir;
	int rval = 0;
	struct stat sb;

	ARGBEGIN {
	case 'f':
		break;
	default:
		wrong(usage);
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		wrong(usage);
	case 2:
		exit(move(argv[0], argv[1]));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &sb) < 0)
		perr(1, "stat %s:", sourcedir);

	if (!S_ISDIR(sb.st_mode))
		perr(1, "%s: not a directory\n", sourcedir);

	for (; *argv != sourcedir; argv++)
		rval |= move(*argv, sourcedir);

	return rval;
}
