#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
rev(FILE *fp)
{
	ssize_t i, len;
	char buf[BUFSIZ];

	while ((len = fgetline(buf, sizeof(buf), fp)) != EOF) {
		i = len - (buf[len] == '\n');
		for (; i--;)
			fputc(buf[i], stdout);
		fputc('\n', stdout);
	}
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int rval;

	rval = 0;
	setprogname(argv[0]);
	argc--, argv++;

	if (!argc)
		rev(stdin);

	for (; *argv; argv++) {
		if (ISDASH(*argv)) {
			*argv = "<stdin>";
			fp    = stdin;
		} else if (!(fp = fopen(*argv, "r"))) {
			warn("fopen %s", *argv);
			rval = 1;
			continue;
		}

		rev(fp);

		if (fp != stdin)
			fclose(fp);
	}

	return rval;
}
