/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char *tty;

	argc--, argv++;

	if (!(tty = ttyname(STDIN_FILENO))) {
		warn("ttyname");
		return 1;
	}

	puts(tty);

	return (fshut("<stdout>", stdout));
}
