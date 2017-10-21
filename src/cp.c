#include <sys/stat.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr,
	    "usage: %s [-fp] [-R [-H|-L|-P]] source target\n"
	    "       %s [-fp] [-R [-H|-L|-P]] source .. dir\n",
	    getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *sourcedir;
	int rval = 0, opts = 0;
	int (*cp)(const char *, const char *, int, int) = copy_file;
	struct stat st;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		opts |= CP_FFLAG;
		break;
	case 'p':
		opts |= CP_PFLAG;
		break;
	case 'r':
	case 'R':
		cp = copy_folder;
		break;
	case 'H':
	case 'L':
	case 'P':
		fs_follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		usage();
	case 2:
		exit(ccii(cp, argv[0], argv[1], opts, 0));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		usage();

	for (; *argv != sourcedir; argc--, argv++)
		rval |= ccii(cp, *argv, sourcedir, opts, 0);

	return rval;
}
