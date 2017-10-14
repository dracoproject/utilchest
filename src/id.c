#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

static int Gguflag;
static int nflag;

static void
printsup(const char *str, uid_t uid)
{
	static int putch;

	if (Gguflag) {
		if (putch++)
			putchar(' ');
		if (nflag)
			printf("%s", str);
		else
			printf("%u", uid);
	} else {
		if (putch++)
			putchar(',');
		printf("%u(%s)", uid, str);
	}
}

static void
group(struct passwd *pw)
{
	struct group *grp;
	unsigned int i;

	setgrent();
	while ((grp = getgrent()) != NULL) {
		if (pw->pw_gid == grp->gr_gid)
			continue;
		for (i = 0; grp->gr_mem[i]; i++)
			if (strcmp(grp->gr_mem[i], pw->pw_name) == 0)
				printsup(grp->gr_name, grp->gr_gid);
	}
	endgrent();
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-G|-g|-u [-nr]] [user]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	int rflag = 0;
	uid_t uid = -1, euid;
	struct group *grp;
	struct passwd *pw;

	setprogname(argv[0]);

	ARGBEGIN {
	case 'G':
	case 'g':
	case 'u':
		if (Gguflag)
			usage();
		Gguflag = ARGC();
		break;
	case 'n':
		nflag = 1;
		break;
	case 'r':
		rflag = 1;
		break;
	default:
		usage();
	} ARGEND

	if (!Gguflag && (nflag || rflag))
		usage();
	if (!Gguflag)
		rflag = 1;

	switch (argc) {
	case 0:
		uid  = getuid();
		break;
	case 1:
		if (isdigit(argv[0][0]))
			uid = stoll(argv[0], 0, UINT_MAX);
		break;
	default:
		usage();
	}

	errno = 0;
	euid  = geteuid();
	if (uid != (uid_t)-1 && !(pw = getpwuid(rflag ? uid : euid))) {
		if (errno)
			err(1, "getpwuid");
		else
			errx(1, "getpwuid %u: no such user", uid);
	}

	if (uid == (uid_t)-1 && !(pw = getpwnam(*argv))) {
		if (errno)
			err(1, "getpwnam %s", *argv);
		else
			errx(1, "getpwnam %s: no such user", *argv);
	}

	if (!(grp = getgrgid(pw->pw_gid)))
		err(1, "getgrgid");

	switch (Gguflag) {
	case 'G':
	case 'g':
		if (nflag)
			printf("%s", grp->gr_name);
		else
			printf("%u", grp->gr_gid);

		if (Gguflag == 'G')
			putchar(' ');
		break;
	case 'u':
		if (nflag)
			printf("%s", pw->pw_name);
		else
			printf("%u", pw->pw_uid);
		break;
	default:
		printf("uid=%u(%s) gid=%u(%s) ",
		    pw->pw_uid, pw->pw_name, pw->pw_gid, grp->gr_name);
		if (!argc && pw->pw_uid != euid) {
			if (!(pw = getpwuid(euid)))
				err(1, "getpwuid");
			printf("euid=%u(%s) ", pw->pw_uid, pw->pw_name);
		}
		printf("groups=");
		break;
	}

	if (Gguflag == 'G' || Gguflag == 0)
		group(pw);
	putchar('\n');

	return 0;
}
