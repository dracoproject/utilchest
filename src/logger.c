#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	unsigned n;
	char buf[LINE_MAX];

	n = 0;
	setprogname(argv[0]);
	argc--, argv++;

	if (!argc) {
		while (fgetline(buf, sizeof(buf), stdin) > 0)
			syslog(0, "%s", buf);
		exit(0);
	}

	for (; *argv; argc--, argv++) {
		n += snprintf(buf+n, sizeof(buf)-n, "%s", *argv);
		if (argc-1)
			n += snprintf(buf+n, sizeof(buf)-n, " ");
	}
	syslog(0, "%s", buf);

	return 0;
}
