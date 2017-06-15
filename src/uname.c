/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/utsname.h>

#include <stdio.h>

#include "util.h"

#define MAC 0x01
#define NOD 0x02
#define REL 0x04
#define SYS 0x08
#define VER 0x10
#define ALL (MAC|NOD|REL|SYS|VER)

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
		print |= ALL;
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
		printf("%s", sys.sysname);
	if (print & NOD)
		printf(" %s", sys.nodename);
	if (print & REL)
		printf(" %s", sys.release);
	if (print & VER)
		printf(" %s", sys.version);
	if (print & MAC)
		printf(" %s", sys.machine);
	putchar('\n');

	return (fshut(stdout, "<stdout>"));
}
