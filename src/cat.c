/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

static void
usage(void) {
	perr(1, "usage: %s [-u] [file ...]\n", argv0);
}

static int
cat(int f, const char *name) {
	char buf[BUFSIZ];
	ssize_t n;

	while ((n = read(fd, buf, sizeof(buf))) > 0)
		if (write(1, buf, n) != n)
			return (pwarn("write %s:", name));

	if (n < 0)
		return (pwarn("read %s:", name));

	return 0;
}

int
main(int argc, char *argv[]) {
	int f, rval = 0;

	ARGBEGIN {
	case 'u':
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		cat(0, "<stdin>");

	for (; *argv; argv++) {
		if ((f = open(*argv, O_RDONLY, 0)) < 0) {
			rval = pwarn("open %s:", *argv);
			continue;
		}

		rval |= cat(f, *argv);

		if (fd != -1 && close(fd) < 0)
			rval = pwarn("close %s:", *argv);
	}

	return rval;
}
