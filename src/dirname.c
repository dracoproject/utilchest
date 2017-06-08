/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <stdio.h>

#include "util.h"

static void
usage(void) {
	perr(1, "usage: %s string\n", argv0);
}

int
main(int argc, char *argv[]) {
	argv0 = *argv, argc--, argv++;

	if (argc != 1)
		usage();

	puts(dirname(*argv));

	return (fshut(stdout, "<stdout>"));
}
