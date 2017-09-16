#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *
emalloc(size_t size)
{
	void *p;

	if (!(p = malloc(size)))
		err(1, "malloc");

	return p;
}

char *
estrdup(const char *s)
{
	char *p;

	if (!(p = strdup(s)))
		err(1, "strdup");

	return p;
}
