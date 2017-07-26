/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <limits.h>

#ifndef PRIO_MIN
#define PRIO_MIN -NZERO
#endif

#ifndef PRIO_MAX
#define PRIO_MAX (NZERO-1)
#endif

extern char *__progname;

#define getprogname( ) __progname
#define setprogname(x)
