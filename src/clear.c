#include <stdio.h>

int
main(void)
{
	fputs("\x1b[2J\x1b[H", stdout);
	return 0;
}
