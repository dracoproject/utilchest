#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
head(const char *sname, FILE *stream, size_t n)
{
 	ssize_t len;
	char buf[LINE_MAX];

	for (; n; n--) {
		if ((len = fgetline(buf, sizeof(buf), stream)) < 0)
			break;

		fwrite(buf, sizeof(char), len, stdout);
	}

	if (ferror(stream))
		err(1, "getline %s", sname);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-n number] [file ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	size_t n;
	int first, rval;

	first =  1;
	n     = 10;
	rval  =  0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'n':
		n = strtobase(EARGF(usage()), 0, LLONG_MAX, 10);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		head("<stdin>", stdin, n);

	for (; *argv; argv++) {
		if (ISDASH(*argv)) {
			*argv = "<stdin>";
			fp    = stdin;
		} else if (!(fp = fopen(*argv, "r"))) {
			warn("fopen %s", *argv);
			rval = 1;
			continue;
		}

		if (argc > 1) {
			printf("%s==> %s <==\n", first ? "" : "\n", *argv);
			first = 0;
		}

		head(*argv, fp, n);

		if (fp != stdin)
			fclose(fp);
	}

	exit(rval);
}
