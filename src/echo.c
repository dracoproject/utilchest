#include <stdio.h>
#include <string.h>

#include "util.h"

int
main(int argc, char *argv[])
{
	int nflag;

	nflag = 0;
	argc--, argv++;

	if (*argv && !strcmp(*argv, "-n"))
		nflag++, argc--, argv++;

	for (; *argv; argc--, argv++) {
		fputs(*argv, stdout);
		if (argc-1)
			putchar(' ');
	}

	if (!nflag)
		putchar('\n');

	return (ioshut());
}
