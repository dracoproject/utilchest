/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s [-p] dir ...";

static int
rmdir_path(const char *p) {
	char *d;

	if (rmdir(p) < 0)
		return -1;

	for (d = dirname((char *)p); *d != '.' && *d != '/'; d = dirname(d))
		if (rmdir(d) < 0)
			return -1;

	return 0;
}

int
main(int argc, char *argv[]) {
	int rval = 0, (*rmdirf)(const char *) = rmdir;

	ARGBEGIN {
	case 'p':
		rmdirf = rmdir_path;
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc)
		wrong(usage);

	for (; *argv; argv++)
		if (rmdirf(*argv) < 0)
			rval |= pwarn("rmdir %s:", *argv);

	return rval;
}
