/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

int
copy_file(const char *src, const char *dest, int opts) {
	char *buf = NULL;
	int sf = -1, tf = -1, rval = 0;
	ssize_t rf = 0;
	struct stat st;
	struct timespec times[2];

	if (CP_F & opts)
		unlink(dest);

	if (lstat(src, &st) < 0)
		return (pwarn("lstat %s:", src));

	if (!(buf = malloc(st.st_size * sizeof(char))))
		perr(1, "malloc:");

	switch ((st.st_mode & S_IFMT)) {
	case S_IFDIR:
		rval = pwarn("%s is a directory.\n");
		goto clean;

		break;
	case S_IFLNK:
		if ((rf = readlink(src, buf, st.st_mode) < 0)) {
			rval = pwarn("readlink %s:", src);
			goto clean;
		}
		buf[rf] = '\0';

		if (st.st_size < rf) {
			rval = pwarn("symlink increased in size\n");
			goto clean;
		}

		if (symlink(buf, dest) < 0) {
			rval = pwarn("symlink %s:", dest);
			goto clean;
		}

		if ((CP_P & opts) && lchown(dest, st.st_uid, st.st_gid) < 0) {
			rval = pwarn("lchown %s:", dest);
			goto clean;
		}

		break;
	case S_IFREG:
		if ((sf = open(src, O_RDONLY, 0)) < 0) {
			rval = pwarn("open %s:", src);
			goto clean;
		}

		if ((tf = open(src, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
			rval = pwarn("open %s:", dest);
			goto clean;
		}

		while ((rf = read(sf, buf, st.st_size)) > 0)
			if (write(tf, buf, rf) != rf) {
				rval = pwarn("write %s:", dest);
				goto clean;
			}

		if (rf < 0) {
			rval = pwarn("read %s:", src);
			goto clean;
		}

		fchmod(tf, st.st_mode);
		if (CP_P & opts) {
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

		break;
	default:
		if (mknod(dest, st.st_mode, st.st_dev) < 0) {
			rval = pwarn("mknod %s:", dest);
			goto clean;
		}

		break;
	}

clean:
	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}

	if ((sf != -1) && (close(sf) < 0))
		rval = pwarn("close %s:", src);

	if ((tf != -1) && (close(tf) < 0))
		rval = pwarn("close %s:", dest);

	return rval;
}
