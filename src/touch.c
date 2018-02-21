#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

enum Flags {
	AFLAG = 0x01,
	CFLAG = 0x02,
	MFLAG = 0x04
};

static struct timespec tms[2] = {{.tv_nsec = UTIME_NOW}};

static int
touch(const char *path, int opts)
{
	struct stat st;
	int fd, rval;

	fd   = -1;
	rval =  0;

	if ((fd = open(path, O_RDONLY, 0)) < 0) {
		if (errno != ENOENT) {
			warn("open %s", path);
			goto failure;
		}
		if (opts & CFLAG)
			goto done;
		if ((fd = creat(path, DEFFILEMODE)) < 0) {
			warn("creat %s", path);
			goto failure;
		}
	}

	if (fstat(fd, &st) < 0) {
		warn("fstat %s", path);
		goto failure;
	}

	tms[0] = (opts & AFLAG) ? tms[0] : st.st_atim;
	tms[1] = (opts & MFLAG) ? tms[1] : st.st_mtim;

	if (futimens(fd, tms) < 0) {
		warn("futimens %s", path);
		goto failure;
	}

	goto done;
failure:
	rval = 1;
done:
	if (fd != -1)
		close(fd);

	return rval;
}

/* TODO: Add fractional seconds */
static time_t
parsetime(char *str, int type)
{
	struct tm lt;
	time_t now;
	int utc, yearset;
	char *dot, fmt[32];

	utc     = 0;
	yearset = 0;
	memset(&lt, 0, sizeof(lt));

	if ((now = time(NULL)) < 0)
		err(1, "time");
	if (!(localtime_r(&now, &lt)))
		err(1, "localtime");

	if ((dot = strchr(str, '.')))
		*dot++ = '\0';

	memset(fmt, 0, sizeof(fmt));
	switch(strlen(str)) {
	case 20:
		if (str[19] != 'Z')
			goto invalid;
		str[19] = '\0';
		utc = 1;
		/* fallthrough */
	case 19:
		strcpy(fmt, "%Y-%m-%dT%H:%M:%S");
		break;
	case 12:
		strcat(fmt, "%Y");
		yearset = 1;
		/* fallthrough */
	case 10:
		if (!yearset)
			strcat(fmt, "%y");
		/* fallthrough */
	case 8:
		strcat(fmt, "%m%d%H%M");
		break;
	default:
		goto invalid;
	}

	if (type == 't' && dot)
		strcat(fmt, ".%S");

	if (!strptime(str, fmt, &lt))
		goto invalid;

	if (utc) {
		lt.tm_hour  += lt.tm_gmtoff / 60;
		lt.tm_gmtoff = 0;
		lt.tm_zone   = "Z";
	}

	return (mktime(&lt));
invalid:
	errno = EINVAL;
	err(1, "parsetime %s", str);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-acm] [-r file|-t time|-d date] file ...\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct stat st;
	int rval, opts;
	char *ref;

	opts = 0;
	ref  = NULL;
	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'a':
		opts |= AFLAG;
		break;
	case 'c':
		opts |= CFLAG;
		break;
	case 'm':
		opts |= MFLAG;
		break;
	case 'r':
		ref = EARGF(usage());
		if (stat(ref, &st) < 0)
			err(1, "stat %s", ref);
		tms[0] = st.st_atim;
		tms[1] = st.st_mtim;
		break;
	case 'd':
	case 't':
		tms[0].tv_sec  = parsetime(EARGF(usage()), ARGC());
		tms[0].tv_nsec = 0;
		break;
	default:
		usage();
	} ARGEND;

	if (!argc)
		usage();
	if (!(opts & (AFLAG|MFLAG)))
		opts |= AFLAG|MFLAG;
	if (!ref)
		tms[1] = tms[0];
	if (~opts & AFLAG)
		tms[0].tv_nsec = UTIME_OMIT;
	if (~opts & MFLAG)
		tms[1].tv_nsec = UTIME_OMIT;

	for (; *argv; argc--, argv++)
		rval |= touch(*argv, opts);

	return rval;
}
