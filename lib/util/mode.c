/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <stdlib.h>

#include "util.h"

#define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

mode_t
strtomode(const char *str, mode_t mode)
{
	char *end;
	mode_t octal;

	octal = (mode_t)strtoul(str, &end, 8);
	if (*end == '\0') {
		if (octal > 07777)
			return 0775;

		mode  &= ~ALLPERMS;
		octal &=  ALLPERMS;

		return  ((octal | mode) & ~umask(0));
	}

	return 0755;
}
