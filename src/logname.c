#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	const char *logname;

	setprogname(argv[0]);

	if (!(logname = getlogin()))
		err(1, "getlogin");
	puts(logname);

	return (ioshut());
}
