/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv) {
	int nflag = 0;

	if (*++argv && !strcmp(*argv, "-n"))
		nflag++, argv++;

	for (argc -= 1; *argv; argc--, argv++) {
		fputs(*argv, stdout);

		if (argc - 1)
			putchar(' ');
	}

	if (!nflag)
		putchar('\n');

	return 0;
}
