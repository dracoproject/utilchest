/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s string [suffix]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *p;
	ssize_t off;

	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 1 && argc != 2)
		usage();

	p = basename(*argv++);
	if (*argv) {
		off = strlen(p) - strlen(*argv);
		if (off > 0 && !strcmp(p + off, *argv))
			p[off] = '\0';
	}
	puts(p);

	exit(0);
}
