#include <sys/file.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-nosux] file command [argument ...]\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	pid_t pid;
	int fd, status;
	int nflag, oflag, opts;

	nflag  = 0;
	oflag  = 0;
	opts   = LOCK_EX;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'n':
		nflag = LOCK_NB;
		break;
	case 'o':
		oflag = 1;
		break;
	case 's':
		opts = LOCK_SH;
		break;
	case 'u':
		opts = LOCK_UN;
		break;
	case 'x':
		opts = LOCK_EX;
		break;
	default:
		usage();
	} ARGEND

	if (argc < 2)
		usage();

	if ((fd = open(*argv, O_RDONLY|O_CREAT, DEFFILEMODE)) < 0)
		err(1, "open %s", *argv);

	argc--, argv++;

	if (flock(fd, opts | nflag) < 0)
		err(1, "flock %s", *argv);

	switch ((pid = fork())) {
	case -1:
		err(1, "fork");
	case  0:
		if (oflag)
			close(fd);

		execvp(*argv, argv);
		err(126 + (errno == ENOENT), "execvp %s", *argv);
	}
	waitpid(pid, &status, 0);
	close(fd);

	if (WIFSIGNALED(status))
		warnx("child killed by signal %d\n", WTERMSIG(status));

	return (WIFEXITED(status) ? WEXITSTATUS(status) : 1);
}
