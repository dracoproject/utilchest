#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

enum Flags {
	FFLAG = 0x1, /* force */
	LFLAG = 0x2, /* follow symlink */
	SFLAG = 0x4 /* create symlink */
};

static int
linkit(const char *src, const char *dest, int opts)
{
	int flags;

	flags = 0;

	if (opts & FFLAG)
		unlink(dest);

	if (opts & LFLAG)
		flags |= AT_SYMLINK_FOLLOW;

	if (opts & SFLAG) {
		if (symlink(src, dest) < 0) {
			warn("symlink %s -> %s", src, dest);
			return 1;
		}
	} else {
		if (linkat(AT_FDCWD, src, AT_FDCWD, dest, flags) < 0) {
			warn("linkat %s -> %s", src, dest);
			return 1;
		}
	}


	return 0;
}

static void
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-f] [-L|-P|-s] source [target]\n"
	        "       %s [-f] [-L|-P|-s] source ... directory\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	int opts, rval;
	char buf[PATH_MAX], *sourcedir;

	opts = 0;
	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		opts |= FFLAG;
		break;
	case 's':
		opts |= SFLAG;
		break;
	case 'L':
		opts |= LFLAG;
		break;
	case 'P':
		opts &= ~LFLAG;
		break;
	default:
		usage();
	} ARGEND

	switch (argc) {
	case 0:
		usage();
	case 1:
		pathcat(buf, argv[0], ".");
		exit(linkit(argv[0], buf, opts));
	case 2:
		pathcat(buf, argv[0], argv[1]);
		exit(linkit(argv[0], buf, opts));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st))
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		usage();

	for (; *argv != sourcedir; argv++) {
		pathcatx(buf, *argv, sourcedir);
		rval |= linkit(*argv, buf, opts);
	}

	return rval;
}
