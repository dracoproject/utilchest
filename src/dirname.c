/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <libgen.h>
#include <stdio.h>

#include "util.h"

SET_USAGE = "%s string";

int
main(int argc, char *argv[])
{
	argc--, argv++;

	if (argc != 1)
		wrong(usage);

	puts(dirname(*argv));

	return (fshut("<stdout>", stdout));
}
