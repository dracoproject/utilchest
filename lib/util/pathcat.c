#include <sys/stat.h>

#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

void
pathcat_(char *buf, size_t bsize, const char *f1, const char *f2)
{
	struct stat st;
	size_t n;
	char *s;

	n = snprintf(buf, bsize, "%s", f2);
	if (!(stat(f2, &st) == 0 && S_ISDIR(st.st_mode)))
		return;

	s = f2[n-1] == '/' ? "" : "/";
	snprintf(buf+n, bsize-n, "%s%s", s, basename((char *)f1));
}

void
pathcatx_(char *buf, size_t bsize, const char *f1, const char *f2)
{
	size_t n;
	char *s;
	n = strlen(f2);
	s = f2[n-1] == '/' ? "" : "/";
	snprintf(buf, bsize, "%s%s%s", f2, s, basename((char *)f1));
}
