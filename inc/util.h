#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <dirent.h>
#include <limits.h>

#include "arg.h"
#include "compat.h"

#define FS_EXEC      1
#define FS_FOLLOW(a) ((fs_follow == 'L') || ((fs_follow == 'H') && !(a)))
#define ISDOT(a)     ((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#define ISDASH(a)    ((a)[0]=='-' && (a)[1]=='\0')

enum cp_flags {
	CP_FFLAG = 0x1, /* force copy */
	CP_PFLAG = 0x2  /* preserve permissions */
};

typedef struct {
	char *dir;
	char *name;
	char path[PATH_MAX];
	DIR *dirp;
	size_t dlen;
	size_t nlen;
	size_t plen;
	struct stat info;
} FS_DIR;

extern int fs_follow;
extern int chown_hflag;

/* call.c */
void pathcat(char *, size_t, const char *, const char *);

/* chown.c */
int chown_file(const char *, uid_t, gid_t, int);
int chown_folder(const char *, uid_t, gid_t, int);

/* cp.c */
int copy_file(const char *, const char *, int, int);
int copy_folder(const char *, const char *, int, int);

/* dir.c */
int open_dir(FS_DIR *, const char *);
int read_dir(FS_DIR *, int);

/* ealloc.c */
void * emalloc(size_t);
char * estrdup(const char *);

/* fshut.c */
int fshut(const char *, FILE *);

/* putstr.c */
void putstr(const char *, FILE *);

/* mode.c */
mode_t strtomode(const char *, mode_t);

/* stoll.c */
long long stoll(const char *, long long, long long);
