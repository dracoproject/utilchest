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
printsp(const char *s, int *space)
{
	if (space++)
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
	int print, space;

	print = 0;
	space = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		print |= AFLAG;
		break;
	case 'm':
		print |= MFLAG;
		break;
	case 'n':
		print |= NFLAG;
		break;
	case 'r':
		print |= RFLAG;
		break;
	case 's':
		print |= SFLAG;
		break;
	case 'v':
		print |= VFLAG;
		break;
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	if (uname(&sys) < 0)
		err(1, "uname");

	if (!print || (print & SFLAG))
		printsp(sys.sysname, &space);
	if (print & NFLAG)
		printsp(sys.nodename, &space);
	if (print & RFLAG)
		printsp(sys.release, &space);
	if (print & VFLAG)
		printsp(sys.version, &space);
	if (print & MFLAG)
		printsp(sys.machine, &space);
	putchar('\n');

	return (ioshut());
}
