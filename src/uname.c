/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/utsname.h>

#include <stdio.h>

#include "util.h"

#define MAC 10000
#define NOD 01000
#define REL 00100
#define SYS 00010
#define VER 00001

static void
usage(void) {
	perr(1, "usage: %s [-amnrsv]\n", argv0);
}

int
main(int argc, char *argv[]) {
	int print = 0;
	struct utsname sys;

	ARGBEGIN {
	case 'a':
		print = MAC|NOD|REL|SYS|VER;
		break;
	case 'm':
		print |= MAC;
		break;
	case 'n':
		print |= NOD;
		break;
	case 'r':
		print |= REL;
		break;
	case 's':
		print |= SYS;
		break;
	case 'v':
		print |= VER;
		break;
	default:
		usage();
	} ARGEND

	if (argc)
		usage();

	if (uname(&sys) < 0)
		perr(1, "uname:");

	if (!print || (print & SYS))
		printf("%s ", sys.sysname);
	if (print & NOD)
		printf("%s ", sys.nodename);
	if (print & REL)
		printf("%s ", sys.release);
	if (print & VER)
		printf("%s ", sys.version);
	if (print & MAC)
		printf("%s ", sys.machine);
	putchar('\n');

	return (fshut(stdout, "<stdout>"));
}
