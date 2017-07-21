/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	struct passwd *pw;

	argv0 = *argv, argc--, argv++;

	if (!(pw = getpwuid(geteuid())))
		perr(1, "gepwuid:");

	puts(pw->pw_name);

	return (fshut("<stdout>", stdout));
}
