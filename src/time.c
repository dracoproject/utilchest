#include <sys/times.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "%s [-p] command [argument ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct tms tms;
	long ticks;
	clock_t btm, etm;
	pid_t pid;
	int status;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'p':
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	if ((ticks = sysconf(_SC_CLK_TCK)) <= 0)
		err(1, "sysconf");

	if ((btm = times(&tms)) < 0)
		err(1, "times");

	switch ((pid = fork())) {
	case -1:
		err(1, "fork");
	case  0:
		execvp(*argv, argv);
		err(126 + (errno == ENOENT), "execvp %s", argv[0]);
	}
	waitpid(pid, &status, 0);

	if (WIFSIGNALED(status))
		warnx("child killed by signal %d\n", WTERMSIG(status));

	if ((etm = times(&tms)) < 0)
		err(1, "times");

	fprintf(stderr,
	        "real %f\nuser %f\nsys %f\n",
	        (etm - btm)    / (double)ticks,
	        tms.tms_cutime / (double)ticks,
	        tms.tms_cstime / (double)ticks);

	return (WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}
