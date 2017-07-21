/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/resource.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

#ifndef PRIO_MIN
#define PRIO_MIN -NZERO
#endif

#ifndef PRIO_MAX
#define PRIO_MAX (NZERO-1)
#endif

SET_USAGE = "%s [-n increment] utility [argument ...]";

int
main(int argc, char *argv[])
{
	int prio = 10;

	ARGBEGIN {
	case 'n':
		prio = estrtonum(EARGF(wrong(usage)), PRIO_MIN, PRIO_MAX);
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc)
		wrong(usage);

	errno = 0;
	prio += getpriority(PRIO_PROCESS, 0);

	if (errno)
		perr(1, "getpriority:");

	if (setpriority(PRIO_PROCESS, 0, prio) < 0)
		perr(1, "setpriority:");

	execvp(*argv, argv);
	pwarn("execvp %s:", argv[0]);

	_exit(126 + (errno == ENOENT));
}
