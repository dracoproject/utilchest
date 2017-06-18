/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s source target";

int
main(int argc, char *argv[]) {
	argv0 = *argv, argc--, argv++;

	if (argc != 2)
		wrong(usage);

	if (link(argv[0], argv[1]) < 0)
		perr(1, "link %s -> %s:", argv[0], argv[1]);

	return 0;
}
