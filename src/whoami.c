#include <err.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	struct passwd *pw;

	setprogname(argv[0]);
	argc--, argv++;

	if (!(pw = getpwuid(geteuid())))
		err(1, "gepwuid");
	puts(pw->pw_name);

	exit(0);
}
