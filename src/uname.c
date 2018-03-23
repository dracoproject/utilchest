#include <sys/utsname.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

enum Flags {
	MFLAG = 0x01,
	NFLAG = 0x02,
	RFLAG = 0x04,
	SFLAG = 0x08,
	VFLAG = 0x10,
	AFLAG = 0x1f
};

static void
printsp(const char *s, int *sp)
{
	if ((*sp)++)
		putchar(' ');
	fputs(s, stdout);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-amnrsv]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct utsname sys;
	int opts, space;

	opts  = 0;
	space = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		opts |= AFLAG;
		break;
	case 'm':
		opts |= MFLAG;
		break;
	case 'n':
		opts |= NFLAG;
		break;
	case 'r':
		opts |= RFLAG;
		break;
	case 's':
		opts |= SFLAG;
		break;
	case 'v':
		opts |= VFLAG;
		break;
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	if (uname(&sys) < 0)
		err(1, "uname");

	if (!opts || (opts & SFLAG))
		printsp(sys.sysname, &space);
	if (opts & NFLAG)
		printsp(sys.nodename, &space);
	if (opts & RFLAG)
		printsp(sys.release, &space);
	if (opts & VFLAG)
		printsp(sys.version, &space);
	if (opts & MFLAG)
		printsp(sys.machine, &space);
	putchar('\n');

	return (ioshut());
}
