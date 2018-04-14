#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "utf.h"
#include "util.h"

#define DATELEN 64

#define NOW        time(NULL)
#define SECSPERDAY (24 * 60 * 60)
#define SIXMONTHS  (180 * SECSPERDAY)

struct column {
	int width;
	int num;
};

struct file {
	char *name;
	char *group;
	char *user;
	char *link;
	mode_t tmode;
	size_t len;
	size_t glen;
	size_t ulen;
	struct file *next;
	struct stat st;
	struct timespec tm;
};

struct max {
	int s_block;
	int s_gid;
	int s_nlink;
	int s_ino;
	int s_size;
	int s_uid;
	int len;
	blkcnt_t block;
	ino_t ino;
	nlink_t nlink;
	off_t size;
	size_t btotal;
	size_t total;
};

static int iflag;
static int lflag;
static int nflag;
static int qflag;
static int rflag;
static int sflag;

static int Aaflag;
static int cuflag;
static int Fpflag;
static int Rdflag;
static int Sftflag;

static int first = 1;
static long blksiz = 512;
static unsigned int termwidth = 80;

static void printc(struct file *, struct max *);
static void (*printfcn)(struct file *, struct max *) = printc;

static int
cmp(struct file *f1, struct file *f2)
{
	int cmp;

	switch (Sftflag) {
	case 'S':
		cmp = f1->st.st_size - f2->st.st_size;
		break;
	case 't':
		if (!(cmp = f1->tm.tv_sec - f2->tm.tv_sec))
			cmp = f1->tm.tv_nsec - f2->tm.tv_nsec;
		break;
	default:
		cmp = strcmp(f1->name, f2->name);
		break;
	}

	return (rflag ? (0 - cmp) : cmp);
}

static void
freefile(struct file *p)
{
	free(p->name);
	free(p->group);
	free(p->link);
	free(p->user);
	free(p);
}

static struct file *
mergelist(struct file *l1, struct file *l2)
{
	struct file *ret;

	ret = NULL;

	if (l1 == NULL)
		return l2;
	if (l2 == NULL)
		return l1;

	if (cmp(l1, l2) <= 0) {
		ret = l1;
		ret->next = mergelist(l1->next, l2);
	} else {
		ret = l2;
		ret->next = mergelist(l1, l2->next);
	}

	return ret;
}

static void
_mergesort(struct file **flist)
{
	struct file *end, *l1, *l2;

	if (!(l1 = *flist) || !(end = (*flist)->next))
		return;

	while (end) {
		if (!(end = end->next))
			break;
		l1 = l1->next;
		end = end->next;
	}

	l2 = l1->next;
	l1->next = NULL;
	l1 = *flist;

	_mergesort(&l1);
	_mergesort(&l2);

	*flist = mergelist(l1, l2);
}

static int
mkcol(struct column *col, struct max *max)
{
	int twidth;

	col->width = max->len;
	if (iflag)
		col->width += max->s_ino + 1;
	if (sflag)
		col->width += max->s_block + 1;
	if (Fpflag)
		col->width += sizeof(char);

	col->width += 1;
	twidth = termwidth + 1;

	if (twidth < (2 * col->width))
		return 1;

	col->num   = (twidth / col->width);
	col->width = (twidth / col->num);

	return 0;
}

static void
mkmax(struct max *max, struct file *file)
{
	char buf[21];

	if (!file) {
		max->s_block = snprintf(buf, sizeof(buf), "%llu",
		                        (unsigned long long)max->block);
		max->s_ino   = snprintf(buf, sizeof(buf), "%llu",
		                        (unsigned long long)max->ino);
		max->s_nlink = snprintf(buf, sizeof(buf), "%lu",
		                        (unsigned long)max->nlink);
		max->s_size  = snprintf(buf, sizeof(buf), "%lld",
		                        (long long)max->size);

		return;
	}

	max->block = MAX(file->st.st_blocks, max->block);
	max->ino   = MAX(file->st.st_ino, max->ino);
	max->len   = MAX(file->len, max->len);
	max->nlink = MAX(file->st.st_nlink, max->nlink);
	max->s_gid = MAX(file->glen, max->s_gid);
	max->s_uid = MAX(file->ulen, max->s_uid);
	max->size  = MAX(file->st.st_size, max->size);

	max->total  += 1;
	max->btotal += file->st.st_blocks;
}

struct file *
newfile(const char *path, const char *str, struct stat *info)
{
	struct file *new;
	struct stat st;
	struct passwd *pw;
	struct group *gr;
	ssize_t len;
	char lp[PATH_MAX], group[32], user[32];

	/* alloc/copy initial values */
	new       = emalloc(1 * sizeof(*new));
	new->name = estrdup(str);
	new->len  = strlen(str);
	new->st   = *info;

	/* set to null to avoid free trash values */
	new->group = NULL;
	new->link  = NULL;
	new->user  = NULL;

	switch (cuflag) {
	case 'c':
		new->tm = new->st.st_ctim;
		break;
	case 'u':
		new->tm = new->st.st_atim;
		break;
	default:
		new->tm = new->st.st_mtim;
		break;
	}

	if (S_ISLNK(new->st.st_mode)) {
		if (stat(path, &st) == 0)
			new->tmode = st.st_mode;
		else
			new->tmode = 0;

		if ((len = readlink(path, lp, sizeof(lp) - 1)) < 0)
			err(1, "readlink %s", path);

		lp[len] = '\0';
		new->link = estrdup(lp);
	}

	if (!lflag)
		return new;

	if (!nflag && (pw = getpwuid(new->st.st_uid)))
		snprintf(user, sizeof(user), "%s", pw->pw_name);
	else
		snprintf(user, sizeof(user), "%d", new->st.st_uid);

	if (!nflag && (gr = getgrgid(new->st.st_gid)))
		snprintf(group, sizeof(group), "%s", gr->gr_name);
	else
		snprintf(group, sizeof(group), "%d", new->st.st_gid);

	new->group = estrdup(group);
	new->user  = estrdup(user);
	new->glen  = strlen(group);
	new->ulen  = strlen(user);

	return new;
}

struct file *
popfile(struct file **p)
{
	struct file *old;

	old = *p;
	*p = old->next;

	return old;
}

static void
pushfile(struct file **p, struct file *new)
{
	new->next = *p;
	*p = new;
}

/* internal print functions */
static int
ptype(mode_t mode)
{
	switch (mode & S_IFMT) {
	case S_IFDIR:
		putchar('/');
		return 1;
	case S_IFIFO:
		putchar('|');
		return 1;
	case S_IFLNK:
		putchar('@');
		return 1;
	case S_IFSOCK:
		putchar('=');
		return 1;
	}

	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		putchar('*');
		return 1;
	}

	return 0;
}

static void
pmode(struct stat *st)
{
	char mode[11];

	memcpy(mode, "?---------", sizeof(mode));

	switch (st->st_mode & S_IFMT) {
	case S_IFBLK:
		mode[0] = 'b';
		break;
	case S_IFCHR:
		mode[0] = 'c';
		break;
	case S_IFDIR:
		mode[0] = 'd';
		break;
	case S_IFIFO:
		mode[0] = 'p';
		break;
	case S_IFLNK:
		mode[0] = 'l';
		break;
	case S_IFREG:
		mode[0] = '-';
		break;
	}

	/* usr */
	if (st->st_mode & S_IRUSR)
		mode[1] = 'r';
	if (st->st_mode & S_IWUSR)
		mode[2] = 'w';
	if (st->st_mode & S_IXUSR)
		mode[3] = 'x';
	if (st->st_mode & S_ISUID)
		mode[3] = (mode[3] == 'x') ? 's' : 'S';

	/* group */
	if (st->st_mode & S_IRGRP)
		mode[4] = 'r';
	if (st->st_mode & S_IWGRP)
		mode[5] = 'w';
	if (st->st_mode & S_IXGRP)
		mode[6] = 'x';
	if (st->st_mode & S_ISGID)
		mode[6] = (mode[6] == 'x') ? 's' : 'S';

	/* other */
	if (st->st_mode & S_IROTH)
		mode[7] = 'r';
	if (st->st_mode & S_IWOTH)
		mode[8] = 'w';
	if (st->st_mode & S_IXOTH)
		mode[9] = 'x';
	if (st->st_mode & S_ISVTX)
		mode[9] = (mode[9] == 'x') ? 't' : 'T';

	printf("%s ", mode);
}

static int
pname(struct file *file, int ino, int size)
{
	Rune rune;
	int chcnt, len;
	const char *ch;

	chcnt = 0;
	len   = 0;

	if (iflag && ino)
		chcnt += printf("%*llu ", ino,
		                (unsigned long long)file->st.st_ino);
	if (sflag && size)
		chcnt += printf("%*lld ", size,
		                howmany((long long)file->st.st_blocks, blksiz));

	for (ch = file->name; *ch; ch += len) {
		len = chartorune(&rune, ch);

		if (!qflag || isprintrune(rune))
			chcnt += fwrite(ch, sizeof(char), len, stdout);
		else
			chcnt += fwrite("?", sizeof(char), 1, stdout);
	}

	if (Fpflag == 'F' || (Fpflag == 'p' && S_ISDIR(file->st.st_mode)))
		chcnt += ptype(file->st.st_mode);

	return chcnt;
}

static void
ptime(struct timespec t)
{
	struct tm *tm;
	char *fmt, buf[DATELEN];

	if (NOW > (t.tv_sec + SIXMONTHS))
		fmt = "%b %d %Y ";
	else
		fmt = "%b %d %H:%M";

	if ((tm = localtime(&t.tv_sec)))
		strftime(buf, sizeof(buf), fmt, tm);
	else
		snprintf(buf, sizeof(buf), "%lld", (long long)t.tv_sec);

	printf("%s ", buf);
}

/* external print functions */
static void
print1(struct file *flist, struct max *max)
{
	struct file *p;

	for (p = flist; p; p = p->next) {
		if (!lflag) {
			pname(p, max->s_ino, max->s_block);
			goto next;
		}

		if (iflag)
			printf("%*llu ", max->s_ino,
			       (unsigned long long)p->st.st_ino);
		if (sflag)
			printf("%*lld ", max->s_block,
			       howmany((long long)p->st.st_blocks, blksiz));

		pmode(&p->st);
		printf("%*lu %-*s %-*s ", max->s_nlink, p->st.st_nlink,
		       max->s_uid, p->user, max->s_gid, p->group);

		if (S_ISBLK(p->st.st_mode) || S_ISCHR(p->st.st_mode))
			printf("%3d, %3d ",
			       major(p->st.st_rdev), minor(p->st.st_rdev));
		else
			printf("%*s%*lld ", 8 - max->s_size, "",
			       max->s_size, (long long)p->st.st_size);

		ptime(p->tm);
		pname(p, 0, 0);

		if (S_ISLNK(p->st.st_mode)) {
			printf(" -> %s", p->link);
			ptype(p->tmode);
		}
next:
		putchar('\n');
	}
}

static void
printc(struct file *flist, struct max *max)
{
	struct file **pa, *p;
	struct column cols;
	int chcnt, num, row;
	int base, col, nrows;

	chcnt = 0;
	row   = 0;
	num   = 0;

	if (mkcol(&cols, max)) {
		print1(flist, max);
		return;
	}

	/* create a array of pointers for random access */
	if (!(pa = malloc(max->total * sizeof(struct file *))))
		err(1, "malloc");

	for (p = flist; p; p = p->next)
		pa[num++] = p;

	nrows = num / cols.num;
	if (num % cols.num)
		nrows += 1;

	for (; row < nrows; row++) {
		for (base = row, col = 0; col < cols.num; col++) {
			chcnt = pname(pa[base], max->s_ino, max->s_block);
			if ((base += nrows) >= num)
				break;
			while (chcnt++ < cols.width)
				putchar(' ');
		}
		putchar('\n');
	}

	free(pa);
	pa = NULL;
}

static void
printm(struct file *flist, struct max *max)
{
	struct file *p;
	int chcnt, width;

	chcnt = 0;
	width = 0;

	if (iflag)
		width += max->s_ino;
	if (sflag)
		width += max->s_block;
	if (Fpflag)
		width += 1;

	for (p = flist; p; p = p->next) {
		if (chcnt > 0) {
			putchar(',');
			if ((chcnt += 3) + width + p->len >= termwidth)
				putchar('\n'), chcnt = 0;
			else
				putchar(' ');
		}

		chcnt += pname(p, max->s_ino, max->s_block);
	}

	putchar('\n');
}

static void
printx(struct file *flist, struct max *max)
{
	struct file *p;
	struct column cols;
	int chcnt, col;

	chcnt = 0;
	col   = 0;

	if (mkcol(&cols, max)) {
		print1(flist, max);
		return;
	}

	for (p = flist; p; p = p->next, col++) {
		if (col >= cols.num) {
			col = 0;
			putchar('\n');
		}

		chcnt = pname(p, max->s_ino, max->s_block);
		while (chcnt++ < cols.width)
			putchar(' ');
	}

	putchar('\n');
}

static void
print_list(struct file **flist, struct max *max)
{
	if (sflag || iflag || lflag)
		printf("total: %lu\n",
		       howmany((long unsigned)max->btotal, blksiz));

	mkmax(max, NULL);
	printfcn(*flist, max);
}

static int
lsdir(const char *path, int more, int depth)
{
	FS_DIR dir;
	struct file *flist, *p;
	struct max max;
	int rd;
	char npath[PATH_MAX];

	flist = NULL;
	memset(&max, 0, sizeof(max));

	switch (open_dir(&dir, path)) {
	case FS_ERR:
		warn("open_dir %s", path);
		return 1;
	case FS_CONT:
		return 0;
	}

	if (more || Rdflag == 'R')
		printf((first-- == 1) ? "%s:\n" : "\n%s:\n", path);

	while ((rd = read_dir(&dir, depth)) == FS_EXEC) {
		if (Aaflag != 'a' && ISDOT(dir.name))
			continue;

		if (!Aaflag && dir.name[0] == '.')
			continue;

		pushfile(&flist, newfile(dir.path, dir.name, &dir.info));
		mkmax(&max, flist);
	}

	if (rd == FS_ERR)
		err(1, "read_dir %s", dir.path);

	if (flist && flist->next && Sftflag != 'f')
		_mergesort(&flist);

	if (max.total)
		print_list(&flist, &max);

	if (Rdflag == 'R') {
		for (p = flist; p; p = p->next) {
			if (ISDOT(p->name))
				continue;
			if (!S_ISDIR(p->st.st_mode))
				continue;
			snprintf(npath, sizeof(npath), "%s/%s", path, p->name);
			lsdir(npath, more, depth+1);
		}
	}

	while (flist)
		freefile(popfile(&flist));

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-1AaCcFfiklmnpqrSstux] [file ...]\n",
	        getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct file *dlist, *flist, *p;
	struct stat st;
	struct max max;
	struct winsize w;
	int kflag, more, rval;
	char *temp;

	dlist = NULL;
	flist = NULL;
	kflag = 0;
	rval  = 0;
	memset(&max, 0, sizeof(max));
	setprogname(argv[0]);

	ARGBEGIN {
	case 'i':
		iflag = 1;
		break;
	case 'k':
		kflag = 1;
		break;
	case 'q':
		qflag = 1;
		break;
	case 'r':
		rflag = 1;
		break;
	case 's':
		sflag = 1;
		break;
	case 'n':
		nflag = 1;
		/* fallthrough */
	case 'l':
		lflag = 1;
		/* fallthrough */
	case '1':
		printfcn = print1;
		break;
	case 'C':
		printfcn = printc;
		break;
	case 'm':
		printfcn = printm;
		break;
	case 'x':
		printfcn = printx;
		break;
	case 'A':
	case 'a':
		Aaflag = ARGC();
		break;
	case 'c':
	case 'u':
		cuflag = ARGC();
		break;
	case 'F':
	case 'p':
		Fpflag = ARGC();
		break;
	case 'R':
	case 'd':
		Rdflag = ARGC();
		break;
	case 'f':
		Rdflag = 0;
		Aaflag = 'a';
		/* fallthrough */
	case 'S':
	case 't':
		Sftflag = ARGC();
		break;
	default:
		usage();
	} ARGEND

	if (!(ioctl(fileno(stdout), TIOCGWINSZ, &w)) && w.ws_col > 0)
		termwidth = w.ws_col;

	if ((printfcn != print1) && lflag)
		lflag = 0;

	if (lflag || sflag) {
		if (!kflag && (temp = getenv("BLOCKSIZE")))
			blksiz = strtobase(temp, 1, LONG_MAX, 10);
		else if (kflag)
			blksiz = 1024;

		blksiz /= 512;
	}

	if (printfcn != print1 && (temp = getenv("COLUMNS")))
		termwidth = strtobase(temp, 0, UINT_MAX, 10);

	if (!argc) {
		argv[0] = ".";
		argv[1] = NULL;
	}

	for (; *argv; argv++) {
		if ((FS_FOLLOW(0) ? stat : lstat)(*argv, &st) < 0) {
			warn("(l)stat %s", *argv);
			rval = 1;
			continue;
		}

		if (Rdflag != 'd' && S_ISDIR(st.st_mode)) {
			pushfile(&dlist, newfile(*argv, *argv, &st));
		} else {
			pushfile(&flist, newfile(*argv, *argv, &st));
			mkmax(&max, flist);
		}
	}

	if (dlist && dlist->next && Sftflag != 'f')
		_mergesort(&dlist);
	if (flist && flist->next && Sftflag != 'f')
		_mergesort(&flist);

	if (max.total) {
		first = 0;
		print_list(&flist, &max);
	}

	for (more = argc > 1, p = dlist; p; p = p->next)
		rval |= lsdir(p->name, more, 0);

	while (flist)
		freefile(popfile(&flist));
	while (dlist)
		freefile(popfile(&dlist));

	return (rval | ioshut());
}

