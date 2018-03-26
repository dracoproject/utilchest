#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-R [-H|-L|-P]] mode file ...\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int (*chmodf)(const char *, mode_t, int), rval;
	mode_t mode;

	chmodf = chmodfile;
	rval   = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'R':
		chmodf = chmoddir;
		break;
	case 'H':
	case 'L':
	case 'P':
		fs_follow = ARGC();
		break;
	case 'r': case 'w': case 'x':
	case 'X': case 's': case 't':
		argv[0]--; /* recover lost char */
		goto done;
	default:
		usage();
	} ARGEND
done:
	if (argc < 2)
		usage();

	mode = strtomode(*argv++, ACCESSPERMS);
	for (; *argv; argv++)
		rval |= chmodf(*argv, mode, 0);

	return rval;
}
