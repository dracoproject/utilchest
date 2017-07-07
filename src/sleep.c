/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <limits.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s time";

int
main(int argc, char *argv[])
{
	unsigned int secs;

	argv0 = *argv, argc--, argv++;

	if (argc != 1)
		wrong(usage);

	for (secs = estrtonum(argv[0], 0, UINT_MAX); secs;)
		secs = sleep(secs);

	return 0;
}
