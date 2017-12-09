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
	int prio = 10, oldprio = 0, rval = 0, who = 0, which = PRIO_PROCESS;
	struct passwd *pw;

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
		prio = stoll(EARGF(usage()), PRIO_MIN, PRIO_MAX);
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
			who = stoll(*argv, 0, INT_MAX);
			break;
		case PRIO_USER:
			errno = 0;
			if ((pw = getpwnam(*argv)))
				who = pw->pw_uid;
			else if (!errno)
				who = stoll(*argv, 0, INT_MAX);
			else
				warn("getpwnam %s", pw);
			if (!who)
				goto err;
			break;
		}
		errno = 0;
		oldprio = getpriority(which, who);
		if (errno) {
			warn("getpriority");
			goto err;
		}
		if (setpriority(which, who, prio + oldprio) < 0) {
			warn("setpriority");
			goto err;
		}

		continue;
err:
		rval = 1;
	}

	return rval;
}
