#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "util.h"

static int
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-m mode] name b|c major minor\n"
	        "       %s [-m mode] name p\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	mode_t mode, type;
	unsigned long major, minor;
	int ret;

	mode = DEFFILEMODE;
	type = 0;
	ret  = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'm':
		mode = strtomode(EARGF(usage()), ACCESSPERMS);
		break;
	default:
		usage();
	} ARGEND

	if (argc < 2 || argv[1][1] != '\0')
		usage();

	switch (argv[1][0]) {
	case 'b':
	case 'c':
		if (argc < 4)
			usage();

		type  = (argv[1][0] == 'b') ? S_IFBLK : S_IFCHR;
		major = stoll(argv[2], 0, ULONG_MAX);
		minor = stoll(argv[3], 0, ULONG_MAX);

		ret =  mknod(argv[0], type|mode, makedev(major, minor));
		break;
	case 'p':
		if (argc != 2)
			usage();

		ret = mknod(argv[0], S_IFIFO|mode, 0);
		break;
	default:
		usage();
	}

	if (ret < 0)
		err(1, "mknod %s", argv[0]);

	return 0;
}
