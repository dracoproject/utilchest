#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-p] [-m mode] dir ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	mode_t dmode, mode;
	int pflag, rval;

	mode  = ACCESSPERMS & ~umask(0);
	dmode = mode|S_IWUSR|S_IXUSR;
	pflag = 0;
	rval  = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	case 'm':
		mode = strtomode(EARGF(usage()), ACCESSPERMS);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argc--, argv++) {
		if (pflag) {
			if (genpath(*argv, dmode, mode) < 0) {
				warn("genpath %s", *argv);
				rval = 1;
			}
		} else if (mkdir(*argv, mode)) {
			warn("mkdir %s", *argv);
			rval = 1;
		}
	}

	return rval;
}
