#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s newroot [command ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	const char *cmd, *cmd_arg, *shell;

	setprogname(argv[0]);
	argc--, argv++;

	if (!argc)
		usage();

	if (!(shell = getenv("SHELL")))
		shell = "/bin/sh";

	if (chroot(*argv) < 0)
		err(1, "chroot %s", *argv);

	if (chdir("/") < 0)
		err(1, "chdir");

	if (argc == 1) {
		cmd = "execvp";
		cmd_arg = argv[1];
		execvp(argv[1], &argv[1]);
	} else {
		cmd = "execlp";
		cmd_arg = shell;
		execlp(shell, shell, "-i", (char *)NULL);
	}

	err(126 + (errno == ENOENT), "%s %s", cmd, cmd_arg);
}
