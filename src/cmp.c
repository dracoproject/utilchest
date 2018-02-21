#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-l|-s] file1 file2\n", getprogname());
	exit(2);
}

int
main(int argc, char *argv[])
{
	FILE *fp[2];
	size_t i, line;
	int ch[2], lsflag, rval;

	lsflag = 0;
	rval   = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'l':
	case 's':
		lsflag = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if (argc != 2)
		usage();

	for (i = 0; i < 2; i++) {
		if (ISDASH(argv[i])) {
			argv[i] = "<stdin>";
			fp[i] = stdin;
		} else if (!(fp[i] = fopen(argv[i], "r"))) {
			if (lsflag != 's')
				warn("fopen %s", argv[i]);
			exit(2);
		}
	}

	for (line = i = 1;; i++) {
		ch[0] = getc(fp[0]);
		ch[1] = getc(fp[1]);

		if (ch[0] == EOF || ch[1] == EOF)
			break;
		if (ch[0] != ch[1]) {
			rval = 1;
			if (lsflag == 'l')
				printf("%zu %o %o\n", i, ch[0], ch[1]);
			else if (!lsflag)
				printf("%s %s differ: char %zu, line %zu\n",
				       argv[0], argv[1], i, line);
		}
		if (ch[1] == '\n')
			line++;
	}

	if ((feof(fp[0]) && !feof(fp[1])) ||
	    (feof(fp[1]) && !feof(fp[0])))
		errx(1, "EOF on %s\n", argv[ch[1] == EOF]);

	exit(rval);
}
