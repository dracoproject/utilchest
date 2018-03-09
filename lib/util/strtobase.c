#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include "util.h"

intmax_t
strtobase(const char *str, intmax_t min, intmax_t max, int base)
{
	intmax_t res;
	char *end;

	errno = 0;
	res   = strtoimax(str, &end, base);

	if (end == str || *end != '\0')
		errno = EINVAL;

	if (res > max || res < min)
		errno = ERANGE;

	if (errno)
		err(1, "strtobase %s", str);

	return res;
}
