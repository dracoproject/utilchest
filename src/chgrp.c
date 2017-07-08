/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <string.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-h] [-R [-H|-L|-P]] group file ...";

int
main(int argc, char *argv[])
{
	gid_t gid = -1;
	int (*chownf)(const char *, uid_t, gid_t, int) = chown_file;
	int rval = 0;
	struct group *grp;

	ARGBEGIN {
	case 'R':
		chownf = chown_folder;
		break;
	case 'H':
	case 'L':
	case 'P':
		fs_follow = ARGC();
		break;
	default:
		wrong(usage);
	} ARGEND

	if (argc < 2)
		wrong(usage);

	errno = 0;
	if ((grp = getgrnam(argv[0])))
		gid = grp->gr_gid;
	else if (!errno)
		gid = estrtonum(argv[0], 0, UINT_MAX);
	else
		perr(1, "getgrnam %s:", argv[0]);

	for (argv++; *argv; argv++)
		rval |= chownf(*argv, (uid_t)-1, gid, 0);

	return rval;
}
