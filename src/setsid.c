#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s command [argument ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (!argc)
		usage();

	if (getpgrp() == getpid()) {
		switch (fork()) {
		case -1:
			err(1, "fork");
		case 0:
			break;
		default:
			return 0;
		}
	}

	if (setsid() < 0)
		err(1, "setsid");

	execvp(*argv, argv);
	err(126 + (errno == ENOENT), "execvp %s", *argv);
}
