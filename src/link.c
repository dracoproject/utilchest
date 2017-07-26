/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s source target";

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 2)
		wrong(usage);

	if (link(argv[0], argv[1]) < 0)
		err(1, "link %s -> %s", argv[0], argv[1]);

	return 0;
}
