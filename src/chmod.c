/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-R [-H|-L|-P]] mode file ...";


static int
chmod_file(const char *s, const char *ms, int depth)
{
	mode_t mode;
	struct stat st;

	if ((FS_FOLLOW(depth) ? stat : lstat)(s, &st) < 0) {
		warn("(l)stat %s", s);
		return 1;
	}

	mode = strtomode(ms, st.st_mode);
	if (chmod(s, mode) < 0) {
		warn("fchmodat %s", s);
		return 1;
	}

	return 0;
}

static int
chmod_folder(const char *s, const char *ms, int depth)
{
	FS_DIR dir;
	int rval = 0;

	if (open_dir(&dir, s) < 0) {
		rval = (errno == ENOTDIR);

		if (rval)
			rval = chmod_file(s, ms, depth);
		else
			warn("open_dir %s", s);

		return rval;
	}

	while (read_dir(&dir, depth) != EOF) {
		if (ISDOT(dir.name))
			continue;

		rval |= chmod_folder(s, ms, depth);
		if (S_ISDIR(dir.info.st_mode))
			rval |= chmod_folder(s, ms, depth+1);
	}

	return rval;
}

int
main(int argc, char *argv[])
{
	const char *modestr;
	int (*chmodf)(const char *, const char *, int) = chmod_file;
	int i, rval = 0;

	setprogname(argv[0]);
	argc--, argv++;

	for (; *argv && (*argv)[0] == '-' && (*argv)[1];
	     argc--, argv++) {
		for (i = 1; (*argv)[i]; i++) {
			switch ((*argv)[i]) {
			case 'R':
				chmodf = chmod_folder;
				break;
			case 'H':
			case 'L':
			case 'P':
				fs_follow = (*argv)[i];
				break;
			case 'r': case 'w': case 'x':
			case 'X': case 's': case 't':
				if (i)
					goto done;
				/* fallthrough */
			case '-':
				if (i && !(*argv)[i + 1]) {
					argc--, argv++;
					goto done;
				}
				/* fallthrough */
			default:
				wrong(usage);
			}
		}
	}

done:
	if (argc < 2)
		wrong(usage);

	modestr = *argv++;
	for (; *argv; argv++)
		rval |= chmodf(*argv, modestr, 0);

	return rval;
}
