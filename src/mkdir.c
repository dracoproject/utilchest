/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static int
mkdirp(const char *path, mode_t dir_mode, mode_t mode)
{
	char *p = (char *)path, c = 0;

	do {
		p += strspn(p, "/");
		p += strcspn(p, "/");

		c = *p;
		*p = '\0';

		if (mkdir(path, (c == '\0') ? mode : dir_mode) < 0\
		    && errno != EEXIST)
			return -1;
	} while ((*p = c) != '\0');

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-p] [-m mode] dir ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int pflag = 0, rval = 0;
	mode_t mode = 0777 & ~umask(0), dir_mode = mode|S_IWUSR|S_IXUSR;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'p':
		pflag = 1;
		break;
	case 'm':
		mode = strtomode(ARGF(), S_IRWXU|S_IRWXG|S_IRWXO);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argc--, argv++) {
		if (pflag)
			rval |= mkdirp(*argv, dir_mode, mode);
		else
			rval |= mkdir(*argv, mode);

		if (rval < 0) {
			warn("mkdir %s", *argv);
			rval = 1;
		}

		if (!rval && mode > 0777 && chmod(*argv, mode) < 0) {
			warn("chmod %s", *argv);
			return 1;
		}
	}

	return rval;
}
