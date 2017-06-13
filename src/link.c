/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <unistd.h>

#include "util.h"

static void
usage(void) {
	perr(1, "usage: %s source target\n", argv0);
}

int
main(int argc, char *argv[]) {
	argv0 = *argv, argc--, argv++;

	if (argc != 2)
		usage();

	if (link(argv[0], argv[1]) < 0)
		perr(1, "link %s -> %s:", argv[0], argv[1]);

	return 0;
}
