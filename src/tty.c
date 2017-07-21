/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char *tty;

	argv0 = *argv, argc--, argv++;

	if (!(tty = ttyname(STDIN_FILENO)))
		return (pwarn("ttyname:"));

	puts(tty);

	return (fshut("<stdout>", stdout));
}
