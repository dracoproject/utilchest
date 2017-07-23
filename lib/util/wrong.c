/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void
wrong(const char *s)
{
	fputs("usage: ", stderr);

	for (; *s; s++) {
		if (*s == '\n' && ++s)
			fputs("\n       ", stderr);

		if (*s == '%' && *++s == 's' && ++s)
			fputs("progname", stderr); /* TODO */

		fputc(*s, stderr);
	}
	fputc('\n', stderr);

	exit(1);
}
