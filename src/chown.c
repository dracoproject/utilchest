#include <err.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-h] [-R [-H|-L|-P]] owner[:group] file ...\n"
	        "       %s [-h] [-R [-H|-L|-P]] :group file ...\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct passwd *pwd;
	struct group *grp;
	gid_t gid;
	uid_t uid;
	int (*chownf)(const char *, uid_t, gid_t, int);
	int rval;
	char *owner, *group;

	chownf = chownfile;
	gid    = -1;
	rval   =  0;
	uid    = -1;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'h':
		chown_hflag = 1;
		break;
	case 'R':
		chownf = chowndir;
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

	owner = argv[0];
	if ((group = strchr(owner, ':')))
		*group++ = '\0';

	if (*owner != '\0') {
		errno = 0;

		if ((pwd = getpwnam(owner)))
			uid = pwd->pw_uid;
		else if (!errno)
			uid = strtobase(owner, 0, UINT_MAX, 10);
		else
			err(1, "getpwnam %s", owner);
	}

	if (group && *group) {
		errno = 0;

		if ((grp = getgrnam(group)))
			gid = grp->gr_gid;
		else if (!errno)
			gid = strtobase(group, 0, UINT_MAX, 10);
		else
			err(1, "getgrnam %s", group);
	}

	if ((uid == (uid_t)-1) && (gid == (gid_t)-1))
		usage();

	for (argv++; *argv; argv++)
		rval |= chownf(*argv, uid, gid, 0);

	return rval;
}
