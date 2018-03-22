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

	if (!(tty = ttyname(STDIN_FILENO)))
		err(1, "ttyname");
	puts(tty);

	return (ioshut());
}
