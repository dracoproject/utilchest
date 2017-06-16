/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <stdio.h>

#include "util.h"

static const char *usage = "string";

int
main(int argc, char *argv[]) {
	argv0 = *argv, argc--, argv++;

	if (argc != 1)
		wrong(usage);

	puts(dirname(*argv));

	return (fshut("<stdout>", stdout));
}
