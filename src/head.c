/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

SET_USAGE = "%s [-n number] [file ...]";

static void
head(const char *fname, FILE *f, size_t n)
{
	char *buf = NULL;
	size_t i = 0, bsize = 0;
 	ssize_t len;

	for (; i < n; i++) {
		if ((len = getline(&buf, &bsize, f)) < 0)
			break;

		fwrite(buf, sizeof(char), len, stdout);
	}

	free(buf);
	buf = NULL;

	if (ferror(f))
		perr(1, "getline %s:", fname);
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int first = 1, rval = 0;
	size_t n = 10;

	ARGBEGIN {
	case 'n':
		n = estrtonum(EARGF(wrong(usage)), 0, LLONG_MAX);
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc)
		head("<stdin>", stdin, n);

	for (; *argv; argv++) {
		if (!strcmp(*argv, "-")) {
			*argv = "<stdin>";
			fp = stdin;
		} else if (!(fp = fopen(*argv, "r"))) {
			rval = pwarn("fopen %s:", *argv);
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

	return (rval | fshut("<stdout>", stdout));
}
