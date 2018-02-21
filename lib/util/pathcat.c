#include <sys/stat.h>

#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

void
pathcat(char *buf, size_t bsize, const char *f1, const char *f2)
{
	struct stat st;
	size_t len;

	if (!(stat(f2, &st) == 0 && S_ISDIR(st.st_mode))) {
		snprintf(buf, bsize, "%s", f2);
		return;
	}

	len = strlen(f2);
	if (f2[len-1] == '/')
		snprintf(buf, bsize, "%s%s", f2, basename((char *)f1));
	else
		snprintf(buf, bsize, "%s/%s", f2, basename((char *)f1));
}
