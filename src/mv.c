#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static int
move(const char *src, const char *dest)
{
	if (!rename(src, dest))
		return 0;

	if (errno == EXDEV)
		return (copy_folder(src, dest, 0, CP_PFLAG));
	else
		warn("rename %s -> %s", src, dest);

	return 1;
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: %s [-f] source target\n"
	    "       %s [-f] source ... dir\n",
	    getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *sourcedir;
	int rval = 0;
	struct stat sb;

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
		exit(cc(move, argv[0], argv[1]));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &sb) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(sb.st_mode))
		usage();

	for (; *argv != sourcedir; argv++)
		rval |= cc(move, *argv, sourcedir);

	return rval;
}
