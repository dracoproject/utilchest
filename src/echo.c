/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	int nflag = 0;

	setprogname(argv[0]);
	argc--, argv++;

	if (*argv && !strcmp(*argv, "-n"))
		nflag++, argc--, argv++;

	for (; *argv; argc--, argv++) {
		fputs(*argv, stdout);
		if (argc-1)
			putchar(' ');
	}

	if (!nflag)
		putchar('\n');

	exit(0);
}
