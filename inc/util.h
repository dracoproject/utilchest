/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#include <sys/types.h>

#include <stdio.h>

#include "arg.h"

#define SET_USAGE static const char *usage

/* estrtonum.c */
long long estrtonum(const char *, long long, long long);

/* fshut.c */
int fshut(const char *, FILE *);

/* putstr.c */
void putstr(const char *, FILE *);

/* strtomode.c */
mode_t strtomode(const char *, mode_t);

/* warn.c */
int pwarn(const char *, ...);
void perr(int, const char *, ...);

/* wrong.c */
void wrong(const char *);
