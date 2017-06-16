/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static const char *usage = "[name]";

int
main(int argc, char *argv[]) {
	char domain[HOST_NAME_MAX+1];

	argv0 = *argv, argc--, argv++;

	if (argc > 1)
		wrong(usage);

	if (argc == 1) {
		if (setdomainname(*argv, strlen(*argv)))
			perr(1, "setdomainname:");
	} else {
		if (getdomainname(domain, sizeof(domain)))
			perr(1, "getdomainname:");

		puts(domain);
	}

	return (fshut("<stdout>", stdout));
}
