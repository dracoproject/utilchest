/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>

#include "fs.h"
#include "util.h"

SET_USAGE = "%s [-h] [-R [-H|-L|-P]] owner[:group] file ...\n"
            "%s [-h] [-R [-H|-L|-P]] :group file ...";

int
main(int argc, char *argv[])
{
	char *owner, *group;
	gid_t gid = -1;
	int (*chownf)(const char *, uid_t, gid_t, int) = chown_file;
	int rval = 0;
	struct group *grp;
	struct passwd *pwd;
	uid_t uid = -1;

	ARGBEGIN {
	case 'h':
		fs_follow = 'H';
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
		wrong(usage);
	} ARGEND

	if (argc < 2)
		wrong(usage);

	owner = argv[0];
	if ((group = strchr(owner, ':')))
		*group++ = '\0';

	if (*owner != '\0') {
		errno = 0;

		if ((pwd = getpwnam(owner)))
			uid = pwd->pw_uid;
		else if (!errno)
			uid = estrtonum(owner, 0, UINT_MAX);
		else
			perr(1, "getpwnam %s:", owner);
	}

	if (group && *group) {
		errno = 0;

		if ((grp = getgrnam(group)))
			gid = grp->gr_gid;
		else if (!errno)
			gid = estrtonum(group, 0, UINT_MAX);
		else
			perr(1, "getgrnam %s:", group);
	}

	if ((uid == (uid_t)-1) && (gid == (gid_t)-1))
		wrong(usage);

	for (argv++; *argv; argv++)
		rval |= chownf(*argv, uid, gid, 0);

	return rval;
}
