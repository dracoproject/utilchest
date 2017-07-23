/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

SET_USAGE = "%s string [suffix]";

int
main(int argc, char *argv[])
{
	char *p;
	ssize_t off;

	argc--, argv++;

	if (argc != 1 && argc != 2)
		wrong(usage);

	p = basename(*argv++);
	if (*argv) {
		off = strlen(p) - strlen(*argv);
		if (off > 0 && !strcmp(p + off, *argv))
			p[off] = '\0';
	}
	puts(p);

	return (fshut("<stdout>", stdout));
}
