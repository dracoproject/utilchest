/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "util.h"

#define SIZE(a, b) (strlen((a)) + strlen((b)) + 2)

char *
pcat(const char *f1, const char *f2, int isdir) {
	static char buf[PATH_MAX];
	struct stat st;

	if (isdir)
		goto cat;

	if (lstat(f2, &st) < 0)
		return ((char *)f2);

	if (!(S_ISDIR(st.st_mode)))
		return ((char *)f2);

cat:
	if (SIZE(f1, f2) >= sizeof(buf))
		return ((char *)f2);

	sprintf(buf, "%s/%s", f2, f1);

	return buf;
}
