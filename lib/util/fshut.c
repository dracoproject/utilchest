/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <err.h>
#include <stdio.h>

#include "util.h"

int
fshut(const char *sname, FILE *stream)
{
	int rval = 0;

	if (fclose(stream)) {
		warn("fclose %s", sname);
		rval = 1;
	}

	return rval;
}
