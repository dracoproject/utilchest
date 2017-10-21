#include <sys/stat.h>

#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

static void
glue(char *buf, size_t bsize, const char *f1, const char *f2)
{
	size_t len;
	struct stat st;

	if (!(stat(f2, &st) == 0 && S_ISDIR(st.st_mode))) {
		snprintf(buf, bsize, "%s", f2);
		return;
	}

	len  = strlen(f2);
	if (f2[len-1] == '/')
		snprintf(buf, bsize, "%s%s",  f2, basename((char *)f1));
	else
		snprintf(buf, bsize, "%s/%s", f2, basename((char *)f1));
}

int
cc(int (*fn)(CC), const char *f1, const char *f2)
{
	char buf[PATH_MAX];
	glue(buf, sizeof(buf), f1, f2);
	return (fn(f1, buf));
}

int
cci(int (*fn)(CCI), const char *f1, const char *f2, int opts)
{
	char buf[PATH_MAX];
	glue(buf, sizeof(buf), f1, f2);
	return (fn(f1, buf, opts));
}

int
ccii(int (*fn)(CCII), const char *f1, const char *f2, int opts, int depth)
{
	char buf[PATH_MAX];
	glue(buf, sizeof(buf), f1, f2);
	return (fn(f1, buf, opts, depth));
}
