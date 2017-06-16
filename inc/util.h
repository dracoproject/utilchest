/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <stdio.h>

#include "arg.h"

/* fshut.c */
int fshut(const char *, FILE *);

/* put.c */
void putstr(const char *, FILE *);

/* warn.c */
int pwarn(const char *, ...);
void perr(int, const char *, ...);

/* wrong.c */
void wrong(const char *);
