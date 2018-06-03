#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

int
concat(int f1, const char *s1, int f2, const char *s2)
{
	ssize_t n;
	char buf[BUFSIZ];

	while ((n = read(f1, buf, sizeof(buf))) > 0) {
		if (write(f2, buf, n) != n) {
			warn("write %s", s2);
			return -2;
		}
	}

	if (n < 0) {
		warn("read %s", s1);
		return -1;
	}

	return 0;
}
