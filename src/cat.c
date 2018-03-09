#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-u] [file ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int fd, rval;

	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'u':
		break;
	default:
		usage();
	} ARGEND

	if (!argc &&
	    concat(STDIN_FILENO, "<stdin>", STDOUT_FILENO, "<stdout>") < 0)
		rval = 1;

	for (; *argv; argc--, argv++) {
		if (ISDASH(*argv)) {
			*argv = "<stdin>";
			fd    = STDIN_FILENO;
		} else if ((fd = open(*argv, O_RDONLY, 0)) < 0) {
			warn("open %s", *argv);
			rval = 1;
			continue;
		}

		switch (concat(fd, *argv, STDOUT_FILENO, "<stdout>")) {
		case -1:
			rval = 1;
		case -2:
			exit(rval);
		}

		if (fd != STDIN_FILENO)
			close(fd);
	}

	return rval;
}
