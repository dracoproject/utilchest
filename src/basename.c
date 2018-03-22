#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s string [suffix]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	ssize_t off;
	char *p;

	setprogname(argv[0]);
	argc--, argv++;

	if (argc != 1 && argc != 2)
		usage();

	p = basename(*argv++);
	if (*argv) {
		off = strlen(p) - strlen(*argv);
		if (off > 0 && !strcmp(p + off, *argv))
			p[off] = '\0';
	}
	puts(p);

	return (ioshut());
}
