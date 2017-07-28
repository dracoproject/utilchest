/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

extern char **environ;

int
main(int argc, char *argv[])
{
	const char *s;

	setprogname(argv[0]);
	argc--, argv++;

	if (argc) {
		if (!(s = getenv(*argv))) {
			warn("getenv %s", *argv);
			return 1;
		}

		puts(s);
	} else {
		for (; *environ; environ++)
			puts(*environ);
	}

	return (fshut("<stdout>", stdout));
}
