/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "util.h"

#define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

mode_t
strtomode(const char *str, mode_t mode)
{
	char *end, op;
	mode_t octal, clear, who = 0, perm = 0;

	octal = (mode_t)strtoul(str, &end, 8);
	if (*end == '\0') {
		if (octal > 07777)
			return 0775;

		mode  &= ~ALLPERMS;
		octal &=  ALLPERMS;

		return  ((octal | mode) & ~umask(0));
	}

who:
	switch (*str) {
	case 'a':
		who |= S_IRWXU|S_ISUID|S_IRWXG|S_ISGID|S_IRWXO;
		break;
	case 'u':
		who |= S_IRWXU|S_ISUID;
		break;
	case 'g':
		who |= S_IRWXG|S_ISGID;
		break;
	case 'o':
		who |= S_IRWXO;
		break;
	default:
		goto op;
	}

	str++;
	goto who;

op:
	clear = who ? who : ALLPERMS;
	who   = who ? who : ~umask(0);

	switch (*str) {
	case '+':
	case '-':
	case '=':
		op = *str++;
		break;
	default:
		errno = EINVAL;
		err(1, "strtomode %c", *str);
	}

copy:
	switch (*str) {
	case 'u':
		if (mode & S_IRUSR)
			perm |= S_IRUSR|S_IRGRP|S_IROTH;
		if (mode & S_IWUSR)
			perm |= S_IWUSR|S_IWGRP|S_IWOTH;
		if (mode & S_IXUSR)
			perm |= S_IXUSR|S_IXGRP|S_IXOTH;
		break;
	case 'g':
		if (mode & S_IRGRP)
			perm |= S_IRUSR|S_IRGRP|S_IROTH;
		if (mode & S_IWGRP)
			perm |= S_IWUSR|S_IWGRP|S_IWOTH;
		if (mode & S_IXGRP)
			perm |= S_IXUSR|S_IXGRP|S_IXOTH;
		break;
	case 'o':
		if (mode & S_IROTH)
			perm |= S_IRUSR|S_IRGRP|S_IROTH;
		if (mode & S_IWOTH)
			perm |= S_IWUSR|S_IWGRP|S_IWOTH;
		if (mode & S_IXOTH)
			perm |= S_IXUSR|S_IXGRP|S_IXOTH;
		break;
	default:
		goto perm;
	}

	str++;
	goto copy;

perm:
	switch (*str) {
	case 'r':
		perm |= S_IRUSR|S_IRGRP|S_IROTH;
		break;
	case 's':
		perm |= S_ISUID|S_ISGID;
		break;
	case 't':
		perm |= S_ISVTX;
		break;
	case 'w':
		perm |= S_IWUSR|S_IWGRP|S_IWOTH;
		break;
	case 'x':
		perm |= S_IXUSR|S_IXGRP|S_IXOTH;
		break;
	case 'X':
		if (S_ISDIR(mode) || mode & (S_IXUSR|S_IXGRP|S_IXOTH))
			perm |= S_IXUSR|S_IXGRP|S_IXOTH;
		break;
	default:
		goto apply;
	}

	str++;
	goto perm;

apply:
	switch (op) {
	case '=':
		mode &= ~clear;
		/* fallthrough */
	case '+':
		mode |= (perm & who);
		break;
	case '-':
		mode &= ~(perm & who);
		break;
	}

	if (*str == ',' && str++)
		goto who;

	return mode;
}
