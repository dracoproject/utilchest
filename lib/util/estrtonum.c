/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <errno.h>
#include <stdlib.h>

#include "util.h"

long long
estrtonum(const char *str, long long min, long long max)
{
	char *end;
	long long ll;

	errno = 0;
	ll = strtoll(str, &end, 10);

	if (end == str || *end != '\0')
		errno = EINVAL;

	if (ll > max || ll < min)
		errno = ERANGE;

	if (errno)
		perr(1, "strtol %s:", str);

	return ll;
}
