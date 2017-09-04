/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/utsname.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#define MAC 0x01
#define NOD 0x02
#define REL 0x04
#define SYS 0x08
#define VER 0x10
#define ALL 0x1f


SET_USAGE = "%s [-amnrsv]";

int
main(int argc, char *argv[])
{
	int space = 0, print = 0;
	struct utsname sys;

	setprogname(argv[0]);

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
		wrong(usage);
	} ARGEND

	if (argc)
		wrong(usage);

	if (uname(&sys) < 0)
		err(1, "uname");

	if (!print || (print & SYS)) {
		space++;
		fputs(sys.sysname, stdout);
	}
	if (print & NOD) {
		if (space++)
			putchar(' ');
		fputs(sys.nodename, stdout);
	}
	if (print & REL) {
		if (space++)
			putchar(' ');
		fputs(sys.release, stdout);
	}
	if (print & VER) {
		if (space++)
			putchar(' ');
		fputs(sys.version, stdout);
	}
	if (print & MAC) {
		if (space++)
			putchar(' ');
		fputs(sys.machine, stdout);
	}
	putchar('\n');

	exit(0);
}
