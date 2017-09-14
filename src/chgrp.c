/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

extern int hflag;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-h] [-R [-H|-L|-P]] group file ...\n",
	    getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	gid_t gid = -1;
	int (*chownf)(const char *, uid_t, gid_t, int) = chown_file;
	int rval = 0;
	struct group *grp;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'h':
		hflag = 1;
		break;
	case 'R':
		chownf = chown_folder;
		break;
	case 'H':
	case 'L':
	case 'P':
		fs_follow = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if (argc < 2)
		usage();

	errno = 0;
	if ((grp = getgrnam(argv[0])))
		gid = grp->gr_gid;
	else if (!errno)
		gid = stoll(argv[0], 0, UINT_MAX);
	else
		err(1, "getgrnam %s", argv[0]);

	for (argv++; *argv; argv++)
		rval |= chownf(*argv, (uid_t)-1, gid, 0);

	return rval;
}
