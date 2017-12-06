#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "util.h"

#define MODE O_WRONLY|O_CREAT

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-ai] [file ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	int *fs, fslen, i;
	int aflag = O_TRUNC, iflag = 0, rval = 0;
	ssize_t n;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		aflag = O_APPEND;
		break;
	case 'i':
		iflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (iflag && signal(SIGINT, SIG_IGN) == SIG_ERR)
		err(1, "signal");

	fslen = argc + 1;
	if (!(fs = malloc(fslen * sizeof(*fs))))
		err(1, "malloc");

	for (i = 0; i < argc; i++)
		if ((fs[i] = open(argv[i], MODE|aflag, DEFFILEMODE)) < 0)
			err(1, "open %s", argv[i]);

	fs[i] = STDOUT_FILENO;

	while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		for (i = 0; i < fslen; i++)
			if (write(fs[i], buf, n) != n)
				err(1, "write %s",
				    (i < fslen) ? argv[i] : "<stdout>");
	}

	if (n < 0)
		err(1, "read <stdin>");

	for (i = 0; i < fslen; i++) {
		if (close(fs[i]) < 0) {
			warn("close %s", argv[i]);
			rval = 1;
		}
	}
	free(fs);
	fs = NULL;

	return rval;
}
