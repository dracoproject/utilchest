#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static const char *modestr;

static int
chmodfile(const char *s, int depth)
{
	struct stat st;
	mode_t mode;

	if ((FS_FOLLOW(depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s", s);
		return 1;
	}

	mode = strtomode(modestr, st.st_mode);
	if (chmod(s, mode) < 0) {
		warn("chmod %s", s);
		return 1;
	}

	return 0;
}

static int
chmoddir(const char *s, int depth)
{
	FS_DIR dir;
	int rd, rval;

	rval = 0;

	if (open_dir(&dir, s) < 0) {
		if (!(rval = errno != ENOTDIR))
			rval = chmodfile(s, depth);
		else
			warn("open_dir %s", s);

		return rval;
	}

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		rval |= chmodfile(dir.path, depth);

		if (S_ISDIR(dir.info.st_mode))
			rval |= chmoddir(dir.path, depth+1);
	}

	if (rd < 0) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	return rval;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-R [-H|-L|-P]] mode file ...\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int (*chmodf)(const char *, int), rval;

	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'R':
		chmodf = chmoddir;
		break;
	case 'H':
	case 'L':
	case 'P':
		fs_follow = ARGC();
		break;
	case 'r': case 'w': case 'x':
	case 'X': case 's': case 't':
		argv[0]--; /* recover lost char */
		goto done;
	default:
		usage();
	} ARGEND
done:
	if (argc < 2)
		usage();

	modestr = *argv++;

	for (; *argv; argv++)
		rval |= chmodf(*argv, 0);

	return rval;
}
