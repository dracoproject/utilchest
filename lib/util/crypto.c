#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "crypto.h"
#include "util.h"

static int
hextodec(int ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	return -1;
}

static int
sumcheck(const char *s1, uint8_t *s2, size_t n)
{
	size_t i;
	int b1, b2;

	if (strlen(s1) < (n*2))
		return -1;

	for (i = 0; i < n; i++) {
		if ((b1 = hextodec(*s1++)) < 0)
			return -1; /* invalid format */
		if ((b2 = hextodec(*s1++)) < 0)
			return -1; /* invalid format */
		if (((b1 << 4) | b2) != s2[i])
			return 1;  /* sum mismatch */
	}

	return 0;
}

static void
sumprint(uint8_t *b, size_t n, const char *f)
{
	size_t i;

	for (i = 0; i < n; i++)
		printf("%02x", b[i]);

	printf(" %s\n", f);
}

static int
sumgen(struct crypto *p, int fd, const char *f)
{
	ssize_t n;
	uint8_t buf[BUFSIZ];

	p->init(p->md);

	while ((n = read(fd, buf, sizeof(buf))) > 0)
		p->process(p->md, buf, n);

	if (n < 0) {
		warn("read %s", f);
		return 1;
	}

	p->done(p->md, p->buf);

	return 0;
}

int
crypto_check(struct crypto *p, FILE *fp, const char *fname)
{
	ssize_t n;
	int fd, rval;
	char *file;
	char buf[LINE_MAX];

	rval =  0;

	while ((n = fgetline(buf, sizeof(buf), fp)) > 0) {
		buf[n-1] = '\0';

		if ((file = strchr(buf, ' ')))
			while (*file == ' ')
				*file++ = '\0';

		if (!file || !(*file)) {
			rval = 1;
			continue;
		}

		if ((fd = open(file, O_RDONLY)) < 0) {
			warn("open %s", file);
			rval = 1;
			continue;
		}

		if (sumgen(p, fd, file) < 0) {
			rval = 1;
			continue;
		}

		switch (sumcheck(buf, p->buf, p->bsiz)) {
		case 0:
			printf("%s: OK\n", file);
			break;
		case 1:
			printf("%s: FAILED\n", file);
			/* fallthrough */
		default:
			rval = 1;
		}

		close(fd);
	}

	return rval;
}

int
crypto_print(struct crypto *p, FILE *fp, const char *fname) {
	if (sumgen(p, fileno(fp), fname))
		return -1;
	sumprint(p->buf, p->bsiz, fname);
	return 0;
}
