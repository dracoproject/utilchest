/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void
wrong(const char *s) {
	fprintf(stdout, "usage: %s %s\n", argv0, s);
	exit(1);
}
