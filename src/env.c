/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

extern char **environ;

SET_USAGE = "%s [-i] [name=value]... [utility [args...]]";

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);

	ARGBEGIN {
	case 'i':
		*environ = NULL;
		break;
	default:
		wrong(usage);
	} ARGEND

	for (; *argv && strchr(*argv, '='); argc--, argv++)
		putenv(*argv);

	if (*argv) {
		execvp(*argv, argv);
		err(126 + (errno == ENOENT), "execvp %s", *argv);
	}

	for (; *environ; environ++)
		puts(*environ);

	return (fshut("<stdout>", stdout));
}
