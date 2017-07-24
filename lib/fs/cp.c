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

#define SIZE(a, b) (strlen((a)) + strlen((b)) + 2)

int
copy_file(const char *src, const char *dest, int opts, int depth)
{
	char buf[BUFSIZ], path[PATH_MAX];
	int sf = -1, tf = -1, rval = 0;
	ssize_t rf = 0;
	struct stat st, st1;
	struct timespec times[2];

	if (CP_FFLAG & opts)
		unlink(dest);

	if ((FS_FOLLOW(depth) ? stat : lstat)(src, &st) < 0) {
		warn("lstat %s", src);
		return 1;
	}

	switch ((st.st_mode & S_IFMT)) {
	case S_IFDIR:
		errno = EISDIR;
		warn("%s", src);
		return 1;
		/*NOTREACHED*/
	case S_IFLNK:
		if ((rf = readlink(src, path, sizeof(path)-1)) < 0) {
			warn("readlink %s", src);
			return 1;
		}
		path[rf] = '\0';

		if (st.st_size < rf) {
			warnx("%s: symlink increased in size\n", src);
			return 1;
		}

		if (symlink(path, dest) < 0) {
			warn("symlink %s", dest);
			return 1;
		}

		if ((CP_PFLAG & opts)
		    && lchown(dest, st.st_uid, st.st_gid) < 0) {
			warn("lchown %s", dest);
			return 1;
		}

		break;
	case S_IFREG:
		if ((sf = open(src, O_RDONLY, 0)) < 0) {
			warn("open %s", src);
			return 1;
		}

		if (fstat(sf, &st1) < 0) {
			warn("fstat %s", src);
			rval = 1;
			goto clean;
		}

		if (st.st_ino != st1.st_ino || st.st_dev != st1.st_dev) {
			warnx("%s: changed between calls", src);
			rval = 1;
			goto clean;
		}

		if ((tf = open(dest, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
			warn("open %s", dest);
			rval = 1;
			goto clean;
		}

		while ((rf = read(sf, buf, sizeof(buf))) > 0) {
			if (write(tf, buf, rf) != rf) {
				warn("write %s", dest);
				rval = 1;
				goto clean;
			}
		}

		if (rf < 0) {
			warn("read %s", src);
			rval = 1;
			goto clean;
		}

		fchmod(tf, st.st_mode);
		if (CP_PFLAG & opts) {
			times[0] = st.st_atim;
			times[1] = st.st_mtim;

			if ((utimensat(AT_FDCWD, dest, times, 0)) < 0) {
				warn("utimensat");
				rval = 1;
				goto clean;
			}

			if ((fchown(tf, st.st_uid, st.st_gid)) < 0) {
				warn("fchown %s", dest);
				rval = 1;
				goto clean;
			}
		}
clean:
		if ((sf != -1) && (close(sf) < 0)) {
			warn("close %s", src);
			rval = 1;
		}

		if ((tf != -1) && (close(tf) < 0)) {
			warn("close %s", dest);
			rval = 1;
		}

		break;
	default:
		if (mknod(dest, st.st_mode, st.st_dev) < 0) {
			warn("mknod %s", dest);
			return 1;
		}

		break;
	}

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

		return rval;
	}

	if (!depth)
		(void)mkdir(dest, 0777);

	while (read_dir(&dir, depth) != EOF) {
		if (ISDOT(dir.name))
			continue;

		if (SIZE(dest, dir.name) >= sizeof(buf)) {
			errno = ENAMETOOLONG;
			warn("sprintf %s/%s", dest, dir.name);
			return 1;
		}

		sprintf(buf, "%s/%s", dest, dir.name);

		if (S_ISDIR(dir.info.st_mode)) {
			if (mkdir(buf, dir.info.st_mode) < 0
			    && errno != EEXIST) {
				warn("mkdir %s", buf);
				return 1;
			}

			rval |= copy_folder(dir.path, buf, opts, depth+1);
		} else
			rval |= copy_file(dir.path, buf, opts, depth);
	}

	return rval;
}
