#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

static int
chmod_file(const char *s, const char *ms, int depth)
{
	mode_t mode;
	struct stat st;

	if ((FS_FOLLOW(depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s", s);
		return 1;
	}

	mode = strtomode(ms, st.st_mode);
	if (chmod(s, mode) < 0) {
		warn("chmod %s", s);
		return 1;
	}

	return 0;
}

static int
chmod_folder(const char *s, const char *ms, int depth)
{
	FS_DIR dir;
	int rd, rval = 0;

	if (open_dir(&dir, s) < 0) {
		if (!(rval = errno != ENOTDIR))
			rval = chmod_file(s, ms, depth);
		else
			warn("open_dir %s", s);

		return rval;
	}

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		rval |= chmod_file(s, ms, depth);

		if (S_ISDIR(dir.info.st_mode))
			rval |= chmod_folder(s, ms, depth+1);
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
	const char *modestr;
	int (*chmodf)(const char *, const char *, int) = chmod_file;
	int rval = 0;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'R':
		chmodf = chmod_folder;
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
		rval |= chmodf(*argv, modestr, 0);

	return rval;
}
