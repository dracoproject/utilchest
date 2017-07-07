/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>

#include "util.h"

int
fshut(const char *sname, FILE *stream)
{
	int rval = 0;

	if (fclose(stream))
		rval = pwarn("fclose %s:", sname);

	return rval;
}
