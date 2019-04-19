#include <err.h>
#include <stdio.h>

#include "util.h"

int
fshut(FILE *s, const char *sn)
{
	int rval;
	rval = 0;
	fflush(s);
	if (ferror(s)) {
		warn("ferror %s", sn);
		rval++;
	}
	fclose(s);
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

