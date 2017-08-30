/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s [-L|-P]";

static char *
getcwd_logical(void)
{
	char *pwd;
	struct stat s_pwd, s_dot;

	if (!(pwd = getenv("PWD")) || *pwd != '/')
		return NULL;

	if (stat(pwd, &s_pwd) < 0 || stat(".", &s_dot) < 0)
		return NULL;

	if (s_pwd.st_dev != s_dot.st_dev || s_pwd.st_ino != s_dot.st_ino)
		return NULL;

	return pwd;
}

int
main(int argc, char *argv[])
{
	const char *cwd;
	int logical = 1;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'L':
		logical = 1;
		break;
	case 'P':
		logical = 0;
		break;
	default:
		wrong(usage);
	} ARGEND

	if (argc)
		wrong(usage);

	if (!(cwd = logical ? getcwd(NULL, 0) : getcwd_logical()))
		err(1, "getcwd");

	puts(cwd);

	exit(0);
}
