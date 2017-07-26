/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

SET_USAGE = "%s [-u] [file ...]";

static int
cat(int f, const char *name)
{
	char buf[BUFSIZ];
	ssize_t n;

	while ((n = read(f, buf, sizeof(buf))) > 0)
		if (write(1, buf, n) != n) {
			warn("write %s", name);
			return 1;
		}

	if (n < 0) {
		warn("read %s", name);
		return 1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int f, rval = 0;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'u':
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc)
		cat(0, "<stdin>");

	for (; *argv; argv++) {
		if (!strcmp(*argv, "-")) {
			*argv = "<stdin>";
			f = STDIN_FILENO;
		} else if ((f = open(*argv, O_RDONLY, 0)) < 0) {
			warn("open %s", *argv);
			rval = 1;
			continue;
		}

		rval |= cat(f, *argv);

		if (f != STDIN_FILENO && close(f) < 0) {
			warn("close %s", *argv);
			rval = 1;
		}
	}

	return rval;
}
