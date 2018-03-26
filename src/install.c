#include <err.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static int
install(const char *src, const char *dest, gid_t gid, mode_t mode, uid_t uid)
{
	if (cpfile(src, dest, CP_FFLAG|CP_PFLAG, 0) ||
	    chownfile(dest, uid, gid, 0)            ||
	    chmodfile(dest, mode, 0))
		return 1;
	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-cds] [-g group] [-m mode] [-o owner] "
	        "source ... target\n", getprogname());
	exit(1);
}
int
main(int argc, char *argv[])
{
	struct stat st;
	struct passwd *pwd;
	struct group *grp;
	gid_t gid;
	mode_t mode;
	uid_t uid;
	int dflag, rval;
	char *p, *sourcedir;
	char buf[PATH_MAX];

	dflag =  0;
	gid   = -1;
	mode  =  0;
	rval  =  0;
	uid   = -1;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'c':
	case 's':
		break;
	case 'd':
		dflag = 1;
		break;
	case 'g':
		p     = EARGF(usage());
		errno = 0;
		if ((grp = getgrnam(p)))
			gid = grp->gr_gid;
		else if (!errno)
			gid = strtobase(p, 0, UINT_MAX, 10);
		else
			err(1, "getgrnam %s", p);
		break;
	case 'm':
		p    = EARGF(usage());
		mode = strtomode(p, ACCESSPERMS);
		break;
	case 'o':
		p     = EARGF(usage());
		errno = 0;
		if ((pwd = getpwnam(p)))
			uid = pwd->pw_uid;
		else if (!errno)
			uid = strtobase(p, 0, UINT_MAX, 10);
		else
			err(1, "getpwnam %s", p);
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	gid = (gid == (gid_t)-1) ? getgid() : gid;
	uid = (uid == (uid_t)-1) ? getuid() : uid;

	if (dflag) {
		mode = mode ? mode : ACCESSPERMS;
		for (; *argv; argc--, argv++) {
			if (genpath(*argv, ACCESSPERMS, mode) < 0) {
				warn("genpath %s", *argv);
				rval = 1;
			}
		}
		exit(rval);
	}

	if (argc < 2)
		exit(install(argv[0], argv[1], gid, mode, uid));

	sourcedir = argv[argc - 1];
	if (stat(sourcedir, &st) < 0)
		err(1, "stat %s", sourcedir);

	if (!S_ISDIR(st.st_mode))
		usage();

	for (; *argv != sourcedir; argc--, argv++) {
		pathcat(buf, sizeof(buf), *argv, sourcedir);
		rval |= install(*argv, sourcedir, gid, mode, uid);
	}

	return rval;
}
