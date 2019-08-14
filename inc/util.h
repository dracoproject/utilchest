#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <inttypes.h>
#include <dirent.h>
#include <limits.h>

#include "arg.h"
#include "compat.h"

#define FS_FOLLOW(a) ((fs_follow == 'L') || ((fs_follow == 'H') && !(a)))
#define ISDOT(a)     ((a)[0]=='.' && ((a)[1]==0 || ((a)[1]=='.' && (a)[2]==0)))
#define ISDASH(a)    ((a)[0]=='-' && (a)[1]=='\0')
#define LEN(a)       (sizeof((a))/sizeof((a)[0]))

enum cp_flags {
	CP_FFLAG = 0x1, /* force copy */
	CP_PFLAG = 0x2  /* preserve permissions */
};

enum fs_ret {
	FS_ERR  = -1, /* an error ocurred */
	FS_OK   =  0, /* end */
	FS_EXEC =  1, /* still running */
	FS_CONT =  1  /* dir already accessed */
};

struct histnode {
	struct histnode *next;
	dev_t dev;
	ino_t ino;
};

typedef struct {
	struct stat info;
	size_t dlen;
	size_t nlen;
	size_t plen;
	DIR *dirp;
	char *dir;
	char *name;
	char path[PATH_MAX];
} FS_DIR;

extern struct histnode *fs_hist;
extern int fs_follow;
extern int chown_hflag;

/* chmod.c */
int chmodfile(const char *, mode_t, int);
int chmoddir(const char *, mode_t, int);

/* chown.c */
int chownfile(const char *, uid_t, gid_t, int);
int chowndir(const char *, uid_t, gid_t, int);

/* concat.c */
int concat(int, const char *, int, const char *);

/* cp.c */
int cpfile(const char *, const char *, int, int);
int cpdir(const char *, const char *, int, int);

/* dir.c */
int  open_dir(FS_DIR *, const char *);
int  read_dir(FS_DIR *);
void close_dir(FS_DIR *);

/* ealloc.c */
void * emalloc(size_t);
char * estrdup(const char *);

/* fgetline.c */
ssize_t fgetline(char *, size_t, FILE *);

/* fshut.c */
int fshut(FILE *, const char *);
int ioshut(void);

/* genpath.c */
int genpath(char *, mode_t, mode_t);

/* putstr.c */
void putstr(const char *, FILE *);

/* mode.c */
mode_t strtomode(const char *, mode_t);

/* pathcat.c */
#define pathcat(a, b, c) pathcat_((a), sizeof((a)), b, c)
#define pathcatx(a, b, c) pathcat_((a), sizeof((a)), b, c)
void pathcat_(char *, size_t, const char *, const char *);
void pathcatx_(char *, size_t, const char *, const char *);

/* stoll.c */
long long strtobase(const char *, long long, unsigned long, int);
