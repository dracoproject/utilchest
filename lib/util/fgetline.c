#include <sys/types.h>

#include <stdio.h>

#include "util.h"

ssize_t
fgetline(char *buf, size_t bsize, FILE *stream)
{
	ssize_t n;
	int c;

	c = 0;
	n = 0;

	for (; n < bsize && c != '\n'; n++) {
		if ((c = getc(stream)) == EOF)
			return (n ? n : EOF);
		buf[n] = c;
	}

	return n;
}
