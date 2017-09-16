#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static int
rmdir_path(const char *p)
{
	char *d;

	if (rmdir(p) < 0)
		return -1;

	for (d = dirname((char *)p); *d != '.' && *d != '/'; d = dirname(d))
		if (rmdir(d) < 0)
			return -1;

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-p] dir ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int rval = 0, (*rmdirf)(const char *) = rmdir;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'p':
		rmdirf = rmdir_path;
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argv++) {
		if (rmdirf(*argv) < 0) {
			warn("rmdir %s", *argv);
			rval = 1;
		}
	}

	return rval;
}
