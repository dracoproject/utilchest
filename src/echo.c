/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <string.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	int nflag = 0;

	if (*++argv && !strcmp(*argv, "-n"))
		nflag++, argv++;

	for (argc -= 1; *argv; argc--, argv++)
		putstr(*argv, stdout);

	if (!nflag)
		putchar('\n');

	return (fshut("<stdout>", stdout));
}
