/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "util.h"

static int perms;

static void
usage(void) {
	perr(1, "usage: %s [-afp] [-R [-H|-L|-P]] source ... dest\n", argv0);
}

static int
cp(const char *src, const char *dest, int rtime) {
	struct stat st;

	if ((FTR_FOLLOW(rtime) ? stat : lstat)(src, &st) < 0)
		return (pwarn("(l)stat %s:", src));

	switch ((st.st_mode & S_IFMT)) {
	case S_IFREG:
		return (copy_file(src, dest, perms));
	case S_IFDIR:
		return (pwarn("%s is a directory.\n"));
	case S_IFLNK:
		return (copy_link(src, dest, perms));
	default:
		return (copy_special(src, dest, perms));
	}

	return 0;
}

static int
cp_r(const char *src, const char *dest, int rtime) {
	char buf[PATH_MAX];
	FTR_DIR dir;
	int rval = 0;

	if (ftr_open(src, &dir) < 0) {
		rval = (errno == ENOTDIR) ? cp(src, dest, 0) :
		       pwarn("ftr_open %s:", src);
		return rval;
	}

	if (!rtime)
		(void)mkdir(dest, 0777);

	while (ftr_read(&dir, 0) != EOF) {
		if (ISDOT(dir.name))
			continue;

		if ((strlen(dest) + dir.nlen + 2) >= sizeof(buf)) {
			errno = ENAMETOOLONG;
			return (pwarn("ftr_read %s/%s:", dest, dir.name));
		}

		sprintf(buf, "%s/%s", dest, dir.name);

		switch ((dir.info.st_mode & S_IFMT)) {
		case S_IFREG:
			rval |= copy_file(dir.path, buf, perms);
			break;
		case S_IFDIR:
			if ((mkdir(buf, dir.info.st_mode) < 0) && (errno != EEXIST))
				rval = pwarn("mkdir %s:", buf);

			rval |= cp_r(dir.path, buf, 1);
			break;
		case S_IFLNK:
			rval |= copy_link(dir.path, buf, perms);
			break;
		default:
			rval |= copy_special(src, dest, perms);
			break;
		}
	}

	return rval;
}

int
main(int argc, char *argv[]) {
	const char *sourcedir;
	int rval = 0, (*copy)(const char *, const char *, int) = cp;
	struct stat st;

	ARGBEGIN {
	case 'a':
		copy = cp_r;
		ftr_follow = 'P';
		perms |= CP_P;
		break;
	case 'f':
		perms |= CP_F;
		break;
	case 'p':
		perms |= CP_P;
		break;
	case 'r':
	case 'R':
		copy = cp_r;
		break;
	case 'H':
	case 'L':
	case 'P':
		ftr_follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	switch (argc) {
	case 0:
	case 1:
		usage();
	case 2:
		exit(copy(argv[0], argv[1], 0));
	}

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		perr(1, "stat %s:", sourcedir);

	if (!S_ISDIR(st.st_mode))
		usage();

	for (; *argv != sourcedir; argc--, argv++)
		rval |= copy(*argv, sourcedir, 0);

	return rval;
}
