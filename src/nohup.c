#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

#define FLAGS O_WRONLY|O_CREAT|O_APPEND
#define MODE  S_IRUSR|S_IWUSR

static void
dup2_nohup_out(void)
{
	const char *home;
	char buf[PATH_MAX], *fn = "nohup.out";
	int fd;

	if ((fd = open(fn, FLAGS, MODE)) < 0) {
		if (!(home = getenv("HOME")))
			err(127, "open %s", fn);
		snprintf(buf, sizeof(buf), "%s/%s", home, fn);
		if ((fd = open(buf, FLAGS, MODE)) < 0)
			err(127, "open %s", buf);
	}

	if (dup2(fd, STDOUT_FILENO) < 0)
		err(127, "dup2");
	close(fd);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s command [argument ...]\n", getprogname());
	exit(127);
}

int
main(int argc, char *argv[])
{
	setprogname(argv[0]);
	argc--, argv++;

	if (!argc)
		usage();

	signal(SIGHUP, SIG_IGN);

	if (isatty(STDOUT_FILENO))
		dup2_nohup_out();
	if (isatty(STDERR_FILENO) && dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
		err(1, "dup2");

	signal(SIGHUP, SIG_IGN);
	execvp(*argv, argv);
	err(126 + (errno == ENOENT), "execvp %s", argv[0]);
}
