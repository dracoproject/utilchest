#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s string\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 1)
		usage();

	puts(dirname(*argv));

	return (ioshut());
}
