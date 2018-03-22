#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static int
progaccess(const char *s)
{
	struct stat st;

	if (stat(s, &st) < 0  || !S_ISREG(st.st_mode) || access(s, X_OK) < 0)
		return 0;

	return 1;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-a] name ...\n", getprogname());
	exit(3);
}

int
main(int argc, char *argv[])
{
	int aflag, i, rval;
	char *path, *p;
	char buf[PATH_MAX];

	aflag = 0;
	rval  = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		aflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (!(path = getenv("PATH")))
		errx(3, "PATH environment variable is not set");

	for (; path; path = p) {
		if ((p = strchr(path, ':')))
			*p++ = '\0';

		for (i = 0; i < argc; i++) {
			if (argv[i] == NULL)
				continue;

			if (strchr(argv[i], '/')) {
				if (progaccess(argv[i]))
					puts(argv[i]);
				argv[i] = NULL;
				continue;
			}

			snprintf(buf, sizeof(buf), "%s/%s", path, argv[i]);
			if (progaccess(buf)) {
				puts(buf);
				if (i < argc-1 && !aflag)
					argv[i] = NULL;
				rval++;
			}
		}
	}

	if (ioshut())
		exit(3);

	return (rval == 0) ? 2 : (rval >= argc) ? 0 : 1;
}
