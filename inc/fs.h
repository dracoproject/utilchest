/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>

#include <dirent.h>

#define CP_FFLAG 0x1 /* force copy */
#define CP_PFLAG 0x2 /* preserve permissions */

#define FS_FOLLOW(a) ((fs_follow == 'L') || ((fs_follow == 'H') && !(a)))
#define ISDOT(a) ((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))

typedef struct {
	DIR *dirp;
	char *dir, *name, *path;
	size_t dlen, nlen, plen;
	struct stat info;
} FS_DIR;

extern int fs_follow;

/* chown.c */
int chown_file(const char *, uid_t, gid_t, int);
int chown_folder(const char *, uid_t, gid_t, int);

/* cp.c */
int copy_file(const char *, const char *, int, int);
int copy_folder(const char *, const char *, int, int);

/* dir.c */
int open_dir(FS_DIR *, const char *);
int read_dir(FS_DIR *, int);

/* pcat.c */
char * pcat(const char *, const char *, int);
