/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.h"

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

	fstat(sf, &st1);

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
	close(sf);
	close(tf);

	return rval;
}

static int
copy_lnk(struct copy *cp)
{
	char path[PATH_MAX];
	int rval = 0;
	ssize_t rl;

	if ((rl = readlink(cp->src, path, sizeof(path)-1)) < 0) {
		warn("readlink %s", cp->src);
		goto failure;
	}
	path[rl] = '\0';

	if (cp->st.st_size < rl) {
		warnx("%s: symlink increased in size\n", cp->src);
		goto failure;
	}

	if (symlink(path, cp->dest) < 0) {
		warn("symlink %s -> %s", cp->dest, path);
		goto failure;
	}

	if ((CP_PFLAG & cp->opts)
	    && lchown(cp->dest, cp->st.st_uid, cp->st.st_gid) < 0) {
		warn("lchown %s", cp->dest);
		goto failure;
	}

	goto done;
failure:
	rval = 1;
done:
	return rval;
}

static int
copy_spc(struct copy *cp)
{
	int rval = 0;

	if (mknod(cp->dest, cp->st.st_mode, cp->st.st_dev) < 0) {
		warn("mknod %s", cp->dest);
		rval = 1;
	}

	return rval;
}

/* external functions */
int
copy_file(const char *src, const char *dest, int opts, int depth)
{
	int rval = 0;
	struct copy cp = {.src = src, .dest = dest, .opts = opts};

	if (CP_FFLAG & opts)
		unlink(dest);

	if ((FS_FOLLOW(depth) ? stat : lstat)(src, &cp.st) < 0) {
		warn("lstat %s", src);
		goto failure;
	}

	switch ((cp.st.st_mode & S_IFMT)) {
	case S_IFDIR:
		errno = EISDIR;
		warn("copy_file %s", src);
		goto failure;
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

	goto done;
failure:
	rval = 1;
done:
	return rval;
}

int
copy_folder(const char *src, const char *dest, int opts, int depth)
{
	char buf[PATH_MAX];
	int rval = 0;
	FS_DIR dir;

	if (open_dir(&dir, src) < 0) {
		rval = (errno == ENOTDIR);

		if (rval)
			rval = copy_file(src, dest, opts, depth);
		else
			warn("open_dir %s", src);

		goto done;
	}

	if (!depth)
		(void)mkdir(dest, 0777);

	while (read_dir(&dir, depth) != EOF) {
		if (ISDOT(dir.name))
			continue;

		snprintf(buf, sizeof(buf), "%s/%s", dest, dir.name);

		if (S_ISDIR(dir.info.st_mode)) {
			if (mkdir(buf, dir.info.st_mode) < 0
			    && errno != EEXIST) {
				warn("mkdir %s", buf);
				goto failure;
			}

			rval |= copy_folder(dir.path, buf, opts, depth+1);
		} else
			rval |= copy_file(dir.path, buf, opts, depth);
	}

	goto done;
failure:
	rval = 1;
done:
	return rval;
}
