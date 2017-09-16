#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	char *tty;

	setprogname(argv[0]);
	argc--, argv++;

	if (!(tty = ttyname(STDIN_FILENO))) {
		warn("ttyname");
		return 1;
	}

	puts(tty);

	exit(0);
}
