#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include "util.h"

long long
strtobase(const char *str, long long min, ulong max, int base)
{
	long long res;
	char *end;

	errno = 0;
	res   = strtoll(str, &end, base);

	if (end == str || *end != '\0')
		errno = EINVAL;

	if (res > max || res < min)
		errno = ERANGE;

	if (errno)
		err(1, "strtobase %s", str);

	return res;
}
