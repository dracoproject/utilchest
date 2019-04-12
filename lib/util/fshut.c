#include <err.h>
#include <stdio.h>

#include "util.h"

int
fshut(FILE *s, const char *sn)
{
	int rval;
	rval = 0;
	fflush(s);
	ferror(s);
	fclose(s);
	if (rval < 0) {
		warn("fshut %s", sn);
		rval = 1;
	}
	return rval;
}

int
ioshut(void)
{
	int rval;
	rval = 0;
	rval |= fshut(stdin, "<stdin>");
	rval |= fshut(stdout, "<stdout>");
	return rval;
}

