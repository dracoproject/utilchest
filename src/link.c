#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s source target\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 2)
		usage();

	if (link(argv[0], argv[1]) < 0)
		err(1, "link %s -> %s", argv[0], argv[1]);

	return 0;
}
