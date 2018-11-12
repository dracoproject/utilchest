#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

enum Flags {
	DFLAG = 0x01,
	QFLAG = 0x02,
	TFLAG = 0x04,
	UFLAG = 0x08
};

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-dqtu] [-p directory] [template]\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	ssize_t n;
	int fd, opts;
	char *pdir, *template, *tmpdir;
	char tmp[PATH_MAX];

	n    = 0;
	opts = 0;
	pdir = NULL;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'd':
		opts |= DFLAG;
		break;
	case 'q':
		opts |= QFLAG;
		break;
	case 't':
		opts |= TFLAG;
		break;
	case 'u':
		opts |= UFLAG;
		break;
	case 'p':
		opts |= TFLAG;
		pdir  = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND

	if (argc > 1)
		usage();

	template = argc ? *argv : "tmp.XXXXXXXXXX";

	if (!argc || opts & TFLAG) {
		if (strchr(template, '/'))
			err(1, "template must not contain directory "
			    "separators in -t mode");

		tmpdir = getenv("TMPDIR");
		if (!tmpdir)
			tmpdir = pdir ? pdir : "/tmp";
		n = snprintf(tmp, sizeof(tmp), "%s", tmpdir);
	}

	if (tmp[n-1] != '/')
		tmp[n++] = '/';

	snprintf(tmp+n, sizeof(tmp)-n, "%s", template);

	if (opts & DFLAG) {
		if (!mkdtemp(tmp)) {
			if (~opts & QFLAG)
				err(1, "mkdtemp");
			exit(1);
		}
		if (opts & UFLAG)
			rmdir(tmp);
	} else {
		if ((fd = mkstemp(tmp)) < 0) {
			if (~opts & QFLAG)
				err(1, "mkstemp");
			exit(1);
		}
		close(fd);
		if (opts & UFLAG)
			unlink(tmp);
	}

	puts(tmp);

	exit(0);
}
