/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s newroot [command ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *shell;

	setprogname(argv[0]);
	argc--, argv++;

	if (!argc)
		usage();

	if (!(shell = getenv("SHELL")))
		shell = "/bin/sh";

	if (chroot(*argv) < 0)
		err(1, "chroot %s", *argv);

	if (chdir("/") < 0)
		err(1, "chdir %s", *argv);

	if (argc == 1)
		execvp(argv[1], &argv[1]);
	else
		execlp(shell, shell, "-i", (char *)NULL);

	err(126 + (errno == ENOENT), "exec%cp %s",
	    argc == 1 ? 'v' : 'l', argc == 1 ? shell : argv[1]);
}
