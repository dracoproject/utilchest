/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <stdlib.h>

#include "util.h"

SET_USAGE = "%s [-m mode] file ...";

int
main(int argc, char *argv[])
{
	int rval = 0;
	mode_t mode = DEFFILEMODE & ~umask(0);

	setprogname(argv[0]);

	ARGBEGIN {
	case 'm':
		mode = strtomode(ARGF(), DEFFILEMODE);
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc)
		wrong(usage);

	for (; *argv; argc--, argv++) {
		if (mkfifo(*argv, mode) < 0) {
			warn("mkfifo %s", *argv);
			rval = 1;
		}
	}

	return rval;
}
