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
copy_file(const char *src, const char *dest, int perms) {
	char *buf = NULL;
	int sf, tf, rval = 0;
	struct stat st;
	struct timespec times[2];

	if (CP_F & perms)
		unlink(dest);

	if ((sf = open(src, O_RDONLY, 0)) < 0)
		return (pwarn("open %s:", src));

	if ((tf = open(dest, O_WRONLY|O_CREAT|O_EXCL, 0)) < 0) {
		rval = pwarn("open %s:", dest);
		goto clean;
	}

	if (fstat(sf, &st) < 0) {
		rval = pwarn("fstat %s:", src);
		goto clean;
	}

	if (!(buf = malloc(st.st_size * sizeof(char))))
		perr(1, "malloc:");

	if (read(sf, buf, st.st_size) < 0) {
		rval = pwarn("read %s:", src);
		goto clean;
	}

	if (write(tf, buf, st.st_size) < 0) {
		rval = pwarn("write %s:", dest);
		goto clean;
	}

	fchmod(tf, st.st_mode);
	if (CP_P & perms) {
		times[0] = st.st_atim, times[1] = st.st_mtim;
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

int
copy_link(const char *src, const char *dest, int perms) {
	char buf[PATH_MAX];
	ssize_t rl;
	struct stat st;

	if (lstat(src, &st) < 0)
		return (pwarn("lstat %s:", src));

	if ((rl = readlink(src, buf, sizeof(buf)-1)) < 0)
		return (pwarn("readlink %s:", src));
	buf[rl] = '\0';

	if (st.st_size < rl)
		return (pwarn("symlink increased in size\n"));

	if (symlink(buf, dest) < 0)
		return (pwarn("symlink %s:", dest));

	if ((CP_P & perms) && lchown(dest, st.st_uid, st.st_gid) < 0)
		return (pwarn("lchown %s:", dest));

	return 0;
}
