#include <sys/types.h>

#include <stdio.h>

ssize_t
fgetline(char *buf, size_t bsize, FILE *stream)
{
	int c;
	ssize_t n;

	for (n = 0; n < bsize; n++) {
		c = fgetc(stream);

		if (c == '\n')
			break;
		if (c == EOF)
			return (n ? n : EOF);

		buf[n] = c;
	}

	return n;
}
