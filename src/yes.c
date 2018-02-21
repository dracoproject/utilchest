#include <stdio.h>

int
main(int argc, char *argv[])
{
	const char *s;

	s = (argc > 1) ? argv[1] : "y";

	while (puts(s) >= 0)
		continue;

	return 1;
}
