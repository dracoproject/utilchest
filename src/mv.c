#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static int
move(const char *src, const char *dest)
{
	if (!rename(src, dest))
		return 0;

	if (errno == EXDEV)
		return (cpdir(src, dest, 0, CP_PFLAG));
	else
		warn("rename %s -> %s", src, dest);

	return 1;
}

static void
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-f] source target\n"
	        "       %s [-f] source ... directory\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct stat sb;
	int rval;
	char *sourcedir, buf[PATH_MAX];

	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'f':
		break;
	default:
		usage();
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		usage();
	case 2:
		pathcat(buf, sizeof(buf), argv[0], argv[1]);
		exit(move(argv[0], buf));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &sb) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(sb.st_mode))
		usage();

	for (; *argv != sourcedir; argv++) {
		pathcat(buf, sizeof(buf), *argv, sourcedir);
		rval |= move(*argv, buf);
	}

	return rval;
}
