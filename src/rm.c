/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include "fs.h"
#include "util.h"

#define FFLAG 0x1 /* Ignore some errors */
#define FTIME 0x2 /* First time running? */

SET_USAGE = "%s [-f] [-Rr] file ...";

static int
rm_file(const char *f, int opts) {
	int rval = 0;
	struct stat st;

	if (lstat(f, &st) < 0) {
		if (!(opts & FFLAG) || errno != ENOENT)
			pwarn("lstat %s:", f);

		return (!(opts & FFLAG));
	}

	if (S_ISDIR(st.st_mode))
		rval = rmdir(f);
	else
		rval = unlink(f);

	if (rval < 0)
		return (pwarn("rm_file %s:", f));

	return 0;
}

static int
rm_folder(const char *f, int opts) {
	int rval = 0;
	FS_DIR dir;

	if (open_dir(f, &dir) < 0) {
		rval = (errno == ENOTDIR) ? rm_file(f, opts) :
		       pwarn("open_dir %s:", f);
		return rval;
	}

	while (read_dir(&dir, (opts & FTIME)) != EOF) {
		if (ISDOT(dir.name))
			continue;

		if (S_ISDIR(dir.info.st_mode))
			rval |= rm_folder(dir.path, opts|FTIME);
		else
			rval |= rm_file(dir.path, opts|FTIME);
	}

	if (rmdir(f) < 0)
		return (pwarn("rmdir %s:", f));

	return rval;
}

int
main(int argc, char *argv[]) {
	int (*rm)(const char *, int) = rm_file;
	int opts = 0, rval = 0;

	ARGBEGIN {
	case 'f':
		opts |= FFLAG;
		break;
	case 'r':
	case 'R':
		rm = rm_folder;
		break;
	default:
		wrong(usage);
	} ARGEND

	if (!argc && !(opts & FFLAG))
		wrong(usage);

	for (; *argv; argv++) {
		if (ISDOT(*argv))
			continue;

		rval |= rm(*argv, opts);
	}

	return rval;
}
