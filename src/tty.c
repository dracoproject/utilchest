/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(void) {
	char *tty;

	if (!(tty = ttyname(STDIN_FILENO)))
		return (pwarn("ttyname:"));

	puts(tty);

	return (fshut("<stdout>", stdout));
}
