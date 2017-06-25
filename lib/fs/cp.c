/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

#define SIZE(a, b) (strlen((a)) + strlen((b)) + 2)

int
copy_file(const char *src, const char *dest, int opts) {
	char buf[BUFSIZ], path[PATH_MAX];
	int sf = -1, tf = -1, rval = 0;
	ssize_t rf = 0;
	struct stat st, st1;
	struct timespec times[2];

	if (CP_FFLAG & opts)
		unlink(dest);

	if ((FS_FOLLOW(CP_FTIME & opts) ? stat : lstat)(src, &st) < 0)
		return (pwarn("lstat %s:", src));

	switch ((st.st_mode & S_IFMT)) {
	case S_IFDIR:
		errno = EISDIR;
		return (pwarn("%s:", src));
		/*NOTREACHED*/
	case S_IFLNK:
		if ((rf = readlink(src, path, sizeof(path)-1)) < 0)
			return (pwarn("readlink %s:", src));
		path[rf] = '\0';

		if (st.st_size < rf)
			return (pwarn("%s: symlink increased in size\n", src));

		if (symlink(path, dest) < 0)
			return (pwarn("symlink %s:", dest));

		if ((CP_PFLAG & opts)
		    && lchown(dest, st.st_uid, st.st_gid) < 0)
			return (pwarn("lchown %s:", dest));

		break;
	case S_IFREG:
		if ((sf = open(src, O_RDONLY, 0)) < 0)
			return (pwarn("open %s:", src));

		if (fstat(sf, &st1) < 0) {
			rval = pwarn("fstat %s:", src);
			goto clean;
		}

		if (st.st_ino != st1.st_ino || st.st_dev != st1.st_dev) {
			rval = pwarn("%s: changed between calls", src);
			goto clean;
		}

		if ((tf = open(dest, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
			rval = pwarn("open %s:", dest);
			goto clean;
		}

		while ((rf = read(sf, buf, sizeof(buf))) > 0)
			if (write(tf, buf, rf) != rf) {
				rval = pwarn("write %s:", dest);
				goto clean;
			}

		if (rf < 0) {
			rval = pwarn("read %s:", src);
			goto clean;
		}

		fchmod(tf, st.st_mode);
		if (CP_PFLAG & opts) {
			times[0] = st.st_atim;
			times[1] = st.st_mtim;

			if ((utimensat(AT_FDCWD, dest, times, 0)) < 0) {
				rval = pwarn("utimensat:");
				goto clean;
			}

			if ((fchown(tf, st.st_uid, st.st_gid)) < 0) {
				rval = pwarn("fchown %s:", dest);
				goto clean;
			}
		}
clean:
		if ((sf != -1) && (close(sf) < 0))
			rval = pwarn("close %s:", src);

		if ((tf != -1) && (close(tf) < 0))
			rval = pwarn("close %s:", dest);

		break;
	default:
		if (mknod(dest, st.st_mode, st.st_dev) < 0)
			return (pwarn("mknod %s:", dest));

		break;
	}

	return rval;
}

int
copy_folder(const char *src, const char *dest, int opts) {
	char buf[PATH_MAX];
	int rval = 0;
	FS_DIR dir;

	if (open_dir(src, &dir) < 0) {
		rval = (errno == ENOTDIR) ? copy_file(src, dest, opts) :
		       pwarn("open_dir %s:", src);
		return rval;
	}

	if (!(CP_FTIME & opts))
		(void)mkdir(dest, 0777);

	while (read_dir(&dir, (opts & CP_FTIME)) != EOF) {
		if (ISDOT(dir.name))
			continue;

		if (SIZE(dest, dir.name) >= sizeof(buf)) {
			errno = ENAMETOOLONG;
			return (pwarn("sprintf %s/%s:", dest, dir.name));
		}

		sprintf(buf, "%s/%s", dest, dir.name);

		if (S_ISDIR(dir.info.st_mode)) {
			if (mkdir(buf, dir.info.st_mode) < 0
			    && errno != EEXIST)
				return (pwarn("mkdir %s:", buf));

			rval |= copy_folder(dir.path, buf, opts|CP_FTIME);
		} else
			rval |= copy_file(dir.path, buf, opts|CP_FTIME);
	}

	return rval;
}
