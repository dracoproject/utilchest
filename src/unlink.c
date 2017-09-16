#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s file\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 1)
		usage();

	if (unlink(*argv) < 0)
		err(1, "unlink %s", *argv);

	return 0;
}
