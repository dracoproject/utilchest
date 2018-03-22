#include <err.h>
#include <stdio.h>

int
fshut(FILE *s, const char *sn)
{
	fflush(s);
	rval |= ferror(s);
	rval |= fclose(s);

	if (rval < 0) {
		warn("fshut %s", sn);
		return 1;
	}

	return 0;
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

