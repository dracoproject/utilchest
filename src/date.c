#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-u] [+format]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct tm *tm;
	time_t tval;
	char *fmt, buf[BUFSIZ];

	setprogname(argv[0]);

	ARGBEGIN {
	case 'u':
		setenv("TZ", "UTC0", 1);
		break;
	default:
		usage();
	} ARGEND

	if (time(&tval) < 0)
		err(1, "time");

	fmt = (*argv && **argv == '+') ? *argv++ : "+%a %b %e %H:%M:%S %Z %Y";

	if (!(tm = localtime(&tval)))
		err(1, "localtime");

	strftime(buf, sizeof(buf), ++fmt, tm);
	puts(buf);

	return (ioshut());
}
