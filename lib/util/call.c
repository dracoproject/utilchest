#include <sys/stat.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

int
call(int (*fn)(const char *, const char *, int),
    const char *s1, const char *s2, int opts)
{
	char buf[PATH_MAX];
	struct stat st;

	if (!(stat(s2, &st) == 0 && S_ISDIR(st.st_mode)))
		return (fn(s1, s2, opts));

	snprintf(buf, sizeof(buf), "%s/%s", s2, s1);

	return (fn(s1, buf, opts));
}
