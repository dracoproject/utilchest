/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"

char *
pcat(const char *f1, const char *f2, int isdir)
{
	static char buf[PATH_MAX];
	struct stat st;

	if (isdir)
		goto concat;

	if (lstat(f2, &st) < 0)
		goto notdir;

	if (!(S_ISDIR(st.st_mode)))
		goto notdir;

concat:
	isdir = 1;
	snprintf(buf, sizeof(buf), "%s/%s", f2, f1);

	goto done;
notdir:
	isdir = 0;
done:
	return (isdir ? buf : f2);
}
