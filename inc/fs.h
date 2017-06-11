/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <dirent.h>

#define CP_F 01
#define CP_P 10

#define FST_FOLLOW(a) ((fst_follow == 'L') || ((fst_follow == 'H') && !(a)))

typedef struct {
	DIR *dirp;
	char *dir, *name, *path;
	size_t dlen, nlen, plen;
	struct stat info;
} FST_DIR;

extern int fst_follow;

/* cp.c */
int copy_file(const char *, const char *, int);
int copy_link(const char *, const char *, int);

/* recurse.c */
int fst_open(const char *, FST_DIR *);
int fst_read(FST_DIR *, int);
