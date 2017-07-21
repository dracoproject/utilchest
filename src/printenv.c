/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	extern char **environ;
	const char *s;

	argv0 = *argv, argc--, argv++;

	if (argc) {
		if (!(s = getenv(*argv)))
			return (pwarn("getenv %s:", *argv));

		puts(s);
	} else {
		for (; *environ; environ++)
			puts(*environ);
	}

	return (fshut("<stdout>", stdout));
}
