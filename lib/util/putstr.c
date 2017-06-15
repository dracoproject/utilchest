/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>

void
putstr(const char *str, FILE *stream) {
	static int rt = 0;

	if (rt++)
		fputc(' ', stream);

	fputs(str, stream);
}
