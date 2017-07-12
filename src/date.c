/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "util.h"

SET_USAGE = "%s [-u] [+format]";
/* mmddhhmm[[cc]yy] TODO */

int
main(int argc, char *argv[])
{
	char *fmt, buf[BUFSIZ];
	struct tm *tm;
	time_t tval;

	ARGBEGIN {
	case 'u':
		setenv("TZ", "UTC0", 1);
		break;
	default:
		wrong(usage);
	} ARGEND

	if (time(&tval) < 0)
		perr(1, "time:");

	if (*argv && **argv == '+')
		fmt = *argv++;
	else
		fmt = "+%a %b %e %H:%M:%S %Z %Y";

	if ((tm = localtime(&tval)))
		strftime(buf, sizeof(buf), ++fmt, tm);
	else
		perr(1, "localtime:");

	puts(buf);

	return 0;
}