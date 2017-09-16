#include <limits.h>

#ifndef ALLPERMS
#define ALLPERMS    (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)
#endif

#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#ifndef PRIO_MIN
#define PRIO_MIN -NZERO
#endif

#ifndef PRIO_MAX
#define PRIO_MAX (NZERO-1)
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif

#ifndef __progname
extern char *__progname;
#endif

#ifndef getprogname
#define getprogname( ) __progname
#endif

#ifndef setprogname
#define setprogname(x) __progname = x
#endif
