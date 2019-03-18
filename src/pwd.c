#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static char *
getpwd(void)
{
	struct stat pwd, dot;
	size_t n;
	char  *s;

	if (!(s = getenv("PWD")) || *s != '/')
		return NULL;

	if ((n = strlen(s)) >= PATH_MAX)
		return NULL;

	if (memmem(s, n, "/./",  sizeof("/./")-1) ||
	    memmem(s, n, "/../", sizeof("/../")-1))
		return NULL;

	if (stat(s, &pwd) < 0 || stat(".", &dot) < 0)
		return NULL;

	if (pwd.st_dev != dot.st_dev ||
	    pwd.st_ino != dot.st_ino)
		return NULL;

	return s;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-L|-P]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int   lpflag;
	char  buf[PATH_MAX];
	char *cwd;

	setprogname(argv[0]);

	lpflag = 'L';

	ARGBEGIN {
	case 'L':
	case 'P':
		lpflag = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	cwd = lpflag == 'L' ? getpwd() : NULL;
	if (!cwd && !(cwd = getcwd(buf, sizeof(buf))))
		err(1, "getcwd");

	puts(cwd);

	return (ioshut());
}
