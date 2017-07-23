/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s file";

int
main(int argc, char *argv[])
{
	argc--, argv++;

	if (argc != 1)
		wrong(usage);

	if (unlink(*argv) < 0)
		err(1, "unlink %s", *argv);

	return 0;
}
