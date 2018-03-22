#include <err.h>
#include <stdio.h>

int
fshut(FILE *s, const char *sn)
{
	int rval;

	rval = 0;

	fflush(s);
	rval |= ferror(s);
	rval |= fclose(s);

	if (rval)
		warn("fshut %s", sn);

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

