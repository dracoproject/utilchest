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
	        "       %s [-fp] [-R [-H|-L|-P]] source ... dir\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	int (*cp)(const char *, const char *, int, int);
	int opts, rval;
	char *sourcedir, buf[PATH_MAX];

	cp   = cpfile;
	opts = 0;
	rval = 0;
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
		cp = cpdir;
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
		pathcat(buf, argv[0], argv[1]);
		exit(cp(argv[0], buf, opts, 0));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		usage();

	for (; *argv != sourcedir; argc--, argv++) {
		pathcatx(buf, *argv, sourcedir);
		rval |= cp(*argv, buf, opts, 0);
	}

	return rval;
}
