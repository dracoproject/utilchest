/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <dirent.h>

#define CP_FFLAG 0x1 /* force copy */
#define CP_PFLAG 0x2 /* preserve permissions */
#define CP_FTIME 0x4 /* first time running? */

#define TFH_FOLLOW(a) ((tfh_follow == 'L') || ((tfh_follow == 'H') && !(a)))
#define ISDOT(a) ((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))

typedef struct {
	DIR *dirp;
	char *dir, *name, *path;
	size_t dlen, nlen, plen;
	struct stat info;
} TFH_DIR;

extern int tfh_follow;

/* chown.c */
int chown_file(const char *, uid_t, gid_t, int);
int chown_folder(const char *, uid_t, gid_t, int);

/* cp.c */
int copy_file(const char *, const char *, int);
int copy_folder(const char *, const char *, int);

/* pcat.c */
char * pcat(const char *, const char *, int);

/* tfh.c */
int tfh_open(const char *, TFH_DIR *);
int tfh_read(TFH_DIR *, int);
