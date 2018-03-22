#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

extern char **environ;

int
main(int argc, char *argv[])
{
	const char *s;

	setprogname(argv[0]);
	argc--, argv++;

	if (argc) {
		if (!(s = getenv(*argv)))
			err(1, "getenv %s", *argv);
		puts(s);
	} else {
		for (; *environ; environ++)
			puts(*environ);
	}

	return (ioshut());
}
