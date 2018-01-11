#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

enum Flags {
	P1FLAG  = 0x01,
	P2FLAG =  0x02
};

static const char *charset =
    "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz"
    "0123456789._-";

static int namemax = NAME_MAX;
static int pathmax = PATH_MAX;

static int
pathchk(char *path, int opts)
{
	char *pathd = NULL, *pe, c;
	int ch, rval = 0;
	size_t len, tlen = 0;
	struct stat st;

	if ((opts & P1FLAG) && !*path) {
		warnx("empty pathname");
		goto err;
	}

	if (!(pathd = strdup(path)))
		err(1, "strdup");

	if (lstat(pathd, &st) < 0 && errno != ENOENT) {
		warn("lstat %s", pathd);
		goto err;
	}

	do {
		path += strspn(path, "/");
		pe    = path + (len = strcspn(path, "/"));

		c   = *pe;
		*pe = '\0';

		if (len > namemax) {
			errno = ENAMETOOLONG;
			warn("%s", pathd);
			goto err;
		}

		if ((opts & P1FLAG) && *path == '-') {
			warnx("%s: component %s has leading '-'",
			    pathd, path);
			goto err;
		}

		if ((opts & P2FLAG)) {
			ch = strspn(path, charset);
			if (path[ch]) {
				warnx("%s: component \"%s\" contains"
				    " non-portable character '%c'",
				    pathd, path, path[ch]);
				goto err;
			}
		}

		path  = pe  + 1;
		tlen += len + 1;
	} while ((*pe = c) != '\0');

	if (tlen > pathmax) {
		errno = ENAMETOOLONG;
		warn("pathchk %s", pathd);
		goto err;
	}

	goto done;
err:
	rval = 1;
done:
	if (pathd) {
		free(pathd);
		pathd = NULL;
	}

	return rval;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-Pp] pathname ...\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int rval = 0, opts = 0;

	ARGBEGIN {
	case 'P':
		opts |= P1FLAG;
		break;
	case 'p':
		opts |= P2FLAG;
		namemax = _POSIX_NAME_MAX;
		pathmax = _POSIX_PATH_MAX;
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argc--, argv++)
		rval |= pathchk(*argv, opts);

	return rval;
}
