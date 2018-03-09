#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#define AFLAG 0x01
#define SFLAG 0x02
#define XFLAG 0x04

static int  rval;
static int  opts;
static long blocksize = 512;

static void
display(const char *path, off_t size)
{
	printf("%lld\t%s\n", howmany((long long)size, blocksize), path);
}

static off_t
dufile(const char *path, int depth)
{
	struct stat st;
	off_t size;

	if ((FS_FOLLOW(depth) ? stat : lstat)(path, &st) < 0) {
		warn("(l)stat %s", path);
		goto err;
	}

	size = st.st_blocks;
	if (!depth || (opts & AFLAG))
		display(path, size);

	goto done;
err:
	rval = 1;
	size = 0;
done:
	return size;
}

static off_t
dudir(const char *path, int depth)
{
	FS_DIR dir;
	struct stat st;
	off_t subtotal;
	int rd;

	subtotal = 0;

	if (open_dir(&dir, path) < 0) {
		if (errno != ENOTDIR) {
			warn("open_dir %s", path);
			goto err;
		}

		return dufile(path, depth);
	}

	if ((FS_FOLLOW(depth) ? stat : lstat)(path, &st) < 0) {
		warn("(l)stat %s", path);
		goto err;
	}

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name) || ((opts & XFLAG) &&
		    st.st_dev != dir.info.st_dev))
			continue;

		subtotal += dufile(dir.path, depth+1);
		if (S_ISDIR(dir.info.st_mode))
			subtotal += dudir(dir.path, depth+1);
	}

	if (!((opts & SFLAG) && depth))
		display(path, subtotal);

	if (rd < 0) {
		warn("read_dir %s", dir.path);
		goto err;
	}

	goto done;
err:
	rval     = 1;
	subtotal = 0;
done:
	return subtotal;
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
	int kflag;
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

	if (!argc)
		dudir(".", 0);

	for (; *argv; argc--, argv++)
		dudir(*argv, 0);

	return rval;
}
