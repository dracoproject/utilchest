/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s [name]";

int
main(int argc, char *argv[])
{
	char domain[HOST_NAME_MAX+1];

	argc--, argv++;

	if (argc > 1)
		wrong(usage);

	if (argc == 1) {
		if (setdomainname(*argv, strlen(*argv)))
			err(1, "setdomainname");
	} else {
		if (getdomainname(domain, sizeof(domain)))
			err(1, "getdomainname");

		puts(domain);
	}

	return (fshut("<stdout>", stdout));
}
