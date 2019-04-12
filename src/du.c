#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

enum Flags {
	AFLAG = 0x01,
	SFLAG = 0x02,
	XFLAG = 0x04
};

static int  opts;
static long blocksize = 512;

#define display(a, b) \
printf("%jd\t%s\n", (b), (a));

static int
dudir(const char *path, int depth, off_t *n)
{
	FS_DIR dir;
	struct stat st;
	off_t sbt;
	int rd;

	switch (open_dir(&dir, path)) {
	case FS_ERR:
		if (errno != ENOTDIR) {
			warn("open_dir %s", path);
			return 1;
		}
		if ((FS_FOLLOW(depth) ? stat : lstat)(path, &st) < 0) {
			warn("(l)stat %s", path);
			return 1;
		}
		sbt = howmany(st.st_blocks, blocksize);
		display(path, sbt);
		/* fallthrough */
	case FS_CONT:
		return 0;
	}

	depth++;
	while ((rd = read_dir(&dir)) == FS_EXEC) {
		if (ISDOT(dir.name) ||
		    ((opts & XFLAG) && st.st_dev != dir.info.st_dev))
			continue;
		sbt = howmany(dir.info.st_blocks, blocksize);
		if (S_ISDIR(dir.info.st_mode)) {
			dudir(dir.path, depth, &sbt);
		} else if (opts & AFLAG) {
			display(dir.path, sbt);
		}
		*n += sbt;
	}
	depth--;

	close_dir(&dir);

	if (rd == FS_ERR) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	if (!depth || !(opts & SFLAG))
		display(path, *n);

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-kx] [-a|-s] [-H|-L] [file ...]\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	off_t n;
	int kflag, rval;
	const char *bsize;

	kflag = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'k':
		kflag     = 1;
		blocksize = 1024;
		break;
	case 'x':
		opts |= XFLAG;
		break;
	case 'a':
		opts |= AFLAG;
		break;
	case 's':
		opts |= SFLAG;
		break;
	case 'H':
	case 'L':
		fs_follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if ((opts & AFLAG) && (opts & SFLAG))
		usage();

	if (!kflag && (bsize = getenv("BLOCKSIZE")))
		blocksize = strtobase(bsize, 1, LONG_MAX, 10);
	blocksize /= 512;

	n = 0;
	rval = 0;

	if (!argc)
		rval = dudir(".", 0, &n);

	for (; *argv; argc--, argv++) {
		n = 0;
		rval |= dudir(*argv, 0, &n);
	}

	return (rval | ioshut());
}
