/* This file is part of the UtilChest from EltaninOS
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

SET_USAGE = "%s [-amnrsv]";

int
main(int argc, char *argv[])
{
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
		wrong(usage);
	} ARGEND

	if (argc)
		wrong(usage);

	if (uname(&sys) < 0)
		perr(1, "uname:");

	if (!print || (print & SYS))
		putstr(sys.sysname, stdout);
	if (print & NOD)
		putstr(sys.nodename, stdout);
	if (print & REL)
		putstr(sys.release, stdout);
	if (print & VER)
		putstr(sys.version, stdout);
	if (print & MAC)
		putstr(sys.machine, stdout);
	putchar('\n');

	return (fshut("<stdout>", stdout));
}
