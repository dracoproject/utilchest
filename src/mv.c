/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-f] source target\n"
    "%s [-f] source ... dir";

static int
move(const char *src, const char *dest, int opts)
{
	if (!rename(src, dest))
		return 0;

	if (errno == EXDEV)
		return (copy_folder(src, dest, 0, CP_PFLAG));
	else
		warn("rename %s -> %s", src, dest);

	return 1;
}

int
main(int argc, char *argv[])
{
	const char *sourcedir;
	int rval = 0;
	struct stat sb;

	setprogname(argv[0]);

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
		exit(call(move, argv[0], argv[1], 0));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &sb) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(sb.st_mode))
		wrong(usage);

	for (; *argv != sourcedir; argv++)
		rval |= call(move, *argv, sourcedir, 0);

	return rval;
}
