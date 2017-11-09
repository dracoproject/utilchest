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
	const char *src;
	const char *dest;
	int opts;
	struct stat st;
};

/* internal functions */
static int
copy_reg(struct copy *cp)
{
	char buf[BUFSIZ];
	int sf = -1, tf = -1, rval = 0;
	ssize_t rf;
	struct stat st1;
	struct timespec times[2];

	if ((sf = open(cp->src, O_RDONLY, 0)) < 0) {
		warn("open %s", cp->src);
		goto failure;
	}

	if (fstat(sf, &st1) < 0) {
		warn("fstat %s", cp->src);
		goto failure;
	}

	if (cp->st.st_ino != st1.st_ino ||
	    cp->st.st_dev != st1.st_dev) {
		warnx("%s: changed between calls\n", cp->src);
		goto failure;
	}

	if ((tf = open(cp->dest, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
		warn("open %s", cp->dest);
		goto failure;
	}

	while ((rf = read(sf, buf, sizeof(buf))) > 0) {
		if (write(tf, buf, rf) != rf) {
			warn("write %s", cp->dest);
			goto failure;
		}
	}

	if (rf < 0) {
		warn("read %s", cp->src);
		goto failure;
	}

	fchmod(tf, st1.st_mode);
	if (CP_PFLAG & cp->opts) {
		times[0] = st1.st_atim;
		times[1] = st1.st_mtim;

		if ((utimensat(AT_FDCWD, cp->dest, times, 0)) < 0) {
			warn("utimensat %s", cp->dest);
			goto failure;
		}

		if ((fchown(tf, st1.st_uid, st1.st_gid)) < 0) {
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
	char path[PATH_MAX];
	ssize_t rl;

	if ((rl = readlink(cp->src, path, sizeof(path)-1)) < 0) {
		warn("readlink %s", cp->src);
		return 1;
	}
	path[rl] = '\0';

	if (cp->st.st_size < rl) {
		warnx("%s: symlink increased in size\n", cp->src);
		return 1;
	}

	if (symlink(path, cp->dest) < 0) {
		warn("symlink %s -> %s", cp->dest, path);
		return 1;
	}

	if ((CP_PFLAG & cp->opts)
	    && lchown(cp->dest, cp->st.st_uid, cp->st.st_gid) < 0) {
		warn("lchown %s", cp->dest);
		return 1;
	}

	return 0;
}

static int
copy_spc(struct copy *cp)
{
	if (mknod(cp->dest, cp->st.st_mode, cp->st.st_dev) < 0) {
		warn("mknod %s", cp->dest);
		return 1;
	}

	return 0;
}

/* external functions */
int
cpfile(const char *src, const char *dest, int opts, int depth)
{
	int rval;
	struct copy cp = {.src = src, .dest = dest, .opts = opts};

	if (CP_FFLAG & opts)
		unlink(dest);

	if ((FS_FOLLOW(depth) ? stat : lstat)(src, &cp.st) < 0) {
		warn("lstat %s", src);
		return 1;
	}

	switch ((cp.st.st_mode & S_IFMT)) {
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

int
cpdir(const char *src, const char *dest, int opts, int depth)
{
	char buf[PATH_MAX];
	int rd, rval = 0;
	FS_DIR dir;

	if (open_dir(&dir, src) < 0) {
		if (!(rval = errno != ENOTDIR))
			rval = cpfile(src, dest, opts, depth);
		else
			warn("open_dir %s", src);

		return rval;
	}

	if (!depth)
		(void)mkdir(dest, 0777);

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (ISDOT(dir.name))
			continue;

		snprintf(buf, sizeof(buf), "%s/%s", dest, dir.name);

		if (S_ISDIR(dir.info.st_mode)) {
			if (mkdir(buf, dir.info.st_mode) < 0
			    && errno != EEXIST) {
				warn("mkdir %s", buf);
				return 1;
			}

			rval |= cpdir(dir.path, buf, opts, depth+1);
		} else
			rval |= cpfile(dir.path, buf, opts, depth);
	}

	if (rd < 0) {
		warn("read_dir %s", dir.path);
		return 1;
	}

	return rval;
}
