/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

char *argv0;

static void
vprint(const char *fmt, va_list ap) {
	int sverrno = errno;

	vfprintf(stderr, fmt, ap);
	if (*fmt && fmt[strlen(fmt)-1] == ':')
		fprintf(stderr, " %s\n", strerror(sverrno));
}

int
pwarn(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);

	return 1;
}

void
perr(int status, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vprint(fmt, ap);
	va_end(ap);

	exit(status);
}
