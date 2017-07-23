/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	struct passwd *pw;

	argc--, argv++;

	if (!(pw = getpwuid(geteuid())))
		err(1, "gepwuid");

	puts(pw->pw_name);

	return (fshut("<stdout>", stdout));
}
