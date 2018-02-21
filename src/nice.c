#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-n increment] command [argument ...]\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int prio;

	prio = 10;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'n':
		prio = stoll(EARGF(usage()), PRIO_MIN, PRIO_MAX);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	errno = 0;
	prio += getpriority(PRIO_PROCESS, 0);

	if (errno)
		err(1, "getpriority");

	if (setpriority(PRIO_PROCESS, 0, prio) < 0)
		err(1, "setpriority");

	execvp(*argv, argv);
	err(126 + (errno == ENOENT), "execvp %s", argv[0]);
}
