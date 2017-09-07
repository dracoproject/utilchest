/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [name]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	char domain[HOST_NAME_MAX+1];

	setprogname(argv[0]);
	argc--, argv++;

	if (argc > 1)
		usage();

	if (argc == 1) {
		if (setdomainname(*argv, strlen(*argv)))
			err(1, "setdomainname");
	} else {
		if (getdomainname(domain, sizeof(domain)))
			err(1, "getdomainname");

		puts(domain);
	}

	exit(0);
}
