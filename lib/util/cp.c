#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

struct copy {
	struct stat *st;
	int opts;
	const char *src;
	const char *dest;
};

/* internal functions */
static int
copy_reg(struct copy *cp)
{
	struct stat st;
	struct timespec times[2];
	int rval, sf, tf;

	rval =  0;
	sf   = -1;
	tf   = -1;

	if ((sf = open(cp->src, O_RDONLY, 0)) < 0) {
		warn("open %s", cp->src);
		goto failure;
	}

	if (fstat(sf, &st) < 0) {
		warn("fstat %s", cp->src);
		goto failure;
	}

	if (cp->st->st_ino != st.st_ino ||
	    cp->st->st_dev != st.st_dev) {
		warnx("%s: changed between calls\n", cp->src);
		goto failure;
	}

	if ((tf = open(cp->dest, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
		warn("open %s", cp->dest);
		goto failure;
	}

	if (concat(sf, cp->src, tf, cp->dest) < 0)
		goto failure;

	fchmod(tf, st.st_mode);
	if (CP_PFLAG & cp->opts) {
		times[0] = st.st_atim;
		times[1] = st.st_mtim;

		if ((utimensat(AT_FDCWD, cp->dest, times, 0)) < 0) {
			warn("utimensat %s", cp->dest);
			goto failure;
		}

		if ((fchown(tf, st.st_uid, st.st_gid)) < 0) {
			warn("fchown %s", cp->dest);
			goto failure;
		}
	}

	goto done;
failure:
	rval = 1;
done:
	if (sf != -1)
		close(sf);
	if (tf != -1)
		close(tf);

	return rval;
}

static int
copy_lnk(struct copy *cp)
{
	ssize_t rl;
	char path[PATH_MAX];

	if ((rl = readlink(cp->src, path, sizeof(path)-1)) < 0) {
		warn("readlink %s", cp->src);
		return 1;
	}
	path[rl] = '\0';

	if (cp->st->st_size < rl) {
		warnx("%s: symlink increased in size\n", cp->src);
		return 1;
	}

	if (symlink(path, cp->dest) < 0) {
		warn("symlink %s -> %s", cp->dest, path);
		return 1;
	}

	if ((CP_PFLAG & cp->opts) &&
	    lchown(cp->dest, cp->st->st_uid, cp->st->st_gid) < 0) {
		warn("lchown %s", cp->dest);
		return 1;
	}

	return 0;
}

static int
copy_spc(struct copy *cp)
{
	if (mknod(cp->dest, cp->st->st_mode, cp->st->st_dev) < 0) {
		warn("mknod %s", cp->dest);
		return 1;
	}

	return 0;
}

static int
afile(const char *src, const char *dest, int opts, int depth, struct stat *st)
{
	struct copy cp;
	int rval;

	cp.src  = src;
	cp.dest = dest;
	cp.opts = opts;
	cp.st   = st;

	if (CP_FFLAG & opts)
		unlink(dest);

	rval = 0;

	switch ((cp.st->st_mode & S_IFMT)) {
	case S_IFDIR:
		errno = EISDIR;
		warn("cpfile %s", src);
		return 1;
	case S_IFLNK:
		rval = copy_lnk(&cp);
		break;
	case S_IFREG:
		rval = copy_reg(&cp);
		break;
	default:
		rval = copy_spc(&cp);
		break;
	}

	return rval;
}

/* external functions */
int
cpfile(const char *src, const char *dest, int opts, int depth)
{
	struct stat st;

	if ((FS_FOLLOW(depth) ? stat : lstat)(src, &st) < 0) {
		warn("lstat %s", src);
		return 1;
	}

	return afile(src, dest, opts, depth, &st);
}

int
cpdir(const char *src, const char *dest, int opts, int depth)
{
	FS_DIR dir;
	int rd, rval;
	char buf[PATH_MAX];

	rval = 0;

	switch (open_dir(&dir, src)) {
	case FS_ERR:
		if (errno != ENOTDIR) {
			warn("open_dir %s", src);
			return 1;
		}
		rval = cpfile(src, dest, opts, depth);
		/* fallthrough */
	case FS_CONT:
		return rval;
	}

	if (!depth)
		mkdir(dest, 0777);

	depth++;
	while ((rd = read_dir(&dir)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		snprintf(buf, sizeof(buf), "%s/%s", dest, dir.name);

		if (S_ISDIR(dir.info.st_mode)) {
			if (mkdir(buf, dir.info.st_mode) < 0
			    && errno != EEXIST) {
				warn("mkdir %s", buf);
				return 1;
			}

			rval |= cpdir(dir.path, buf, opts, depth);
		} else {
			rval |= afile(dir.path, buf, opts, depth, &dir.info);
		}
	}
	depth--;

	close_dir(&dir);

	if (rd == FS_ERR) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	return rval;
}
