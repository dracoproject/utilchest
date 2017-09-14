/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s time\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	unsigned int secs;

	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 1)
		usage();

	for (secs = stoll(argv[0], 0, UINT_MAX); secs;)
		secs = sleep(secs);

	return 0;
}
