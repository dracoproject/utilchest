/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s file";

int
main(int argc, char *argv[]) {
	argv0 = *argv, argc--, argv++;

	if (argc != 1)
		wrong(usage);

	if (unlink(*argv))
		perr(1, "unlink %s:", *argv);

	return 0;
}
