#include <sys/resource.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-g|-p|-u] -n increment ID ...\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct passwd *pw;
	int oldprio, prio, rval, which, who;

	oldprio =  0;
	prio    = 10;
	rval    =  0;
	which   = PRIO_PROCESS;
	who     =  0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'g':
		which = PRIO_PGRP;
		break;
	case 'p':
		which = PRIO_PROCESS;
		break;
	case 'u':
		which = PRIO_USER;
		break;
	case 'n':
		prio = strtobase(EARGF(usage()), PRIO_MIN, PRIO_MAX, 10);
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();

	for (; *argv; argc--, argv++) {
		switch (which) {
		case PRIO_PGRP:
		case PRIO_PROCESS:
			who = strtobase(*argv, 0, INT_MAX, 10);
			break;
		case PRIO_USER:
			errno = 0;
			if ((pw = getpwnam(*argv)))
				who = pw->pw_uid;
			else if (!errno)
				who = strtobase(*argv, 0, INT_MAX, 10);
			else
				warn("getpwnam %s", *argv);
			if (!who) {
				rval = 1;
				continue;
			}
			break;
		}

		errno = 0;
		oldprio = getpriority(which, who);

		if (errno) {
			warn("getpriority");
			rval = 1;
			continue;
		}

		if (setpriority(which, who, prio + oldprio) < 0) {
			warn("setpriority");
			rval = 1;
			continue;
		}
	}

	return rval;
}
