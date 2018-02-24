#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-ai] [file ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	ssize_t n;
	mode_t mode;
	int *fds, fdslen, i, rval;
	char buf[BUFSIZ];

	mode = O_WRONLY|O_CREAT|O_TRUNC;
	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		mode = O_WRONLY|O_CREAT|O_APPEND;
		break;
	case 'i':
		signal(SIGINT, SIG_IGN);
		break;
	default:
		usage();
	} ARGEND

	fdslen = argc + 1;
	if (!(fds = malloc(fdslen * sizeof(*fds))))
		err(1, "malloc");

	for (i = 0; i < argc; i++)
		if ((fds[i] = open(argv[i], mode, DEFFILEMODE)) < 0)
			err(1, "open %s", argv[i]);

	fds[i] = STDOUT_FILENO;

	while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		for (i = 0; i < fdslen; i++)
			if (write(fds[i], buf, n) != n)
				err(1, "write %s",
				    (i < fdslen) ? argv[i] : "<stdout>");
	}

	if (n < 0)
		err(1, "read <stdin>");

	for (i = 0; i < fdslen; i++) {
		if (close(fds[i]) < 0) {
			warn("close %s", argv[i]);
			rval = 1;
		}
	}
	free(fds);

	return rval;
}
