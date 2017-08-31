/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
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

#include "fs.h"
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

static int iflag = 0;
static int lflag = 0;
static int nflag = 0;
static int qflag = 0;
static int rflag = 0;
static int sflag = 0;

static int Aaflag = 0;
static int cuflag = 0;
static int Fpflag = 0;
static int Rdflag = 0;
static int Sftflag = 0;

static int first = 1;
static long blocksize = 512;
static unsigned int termwidth = 80;

static void printc(struct file *, struct max *);
static void (*printfcn)(struct file *, struct max *) = printc;

SET_USAGE = "%s [-1AaCcFfiklmnpqrSstux] [file ...]";

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
	free(p->user);
	free(p);
}

/* mergesort copied from heirloom */
static void
_mergesort(struct file **flist)
{
	struct file *l1, *l2, **mid;

	l1 = *(mid = &(*flist)->next);

	do {
		if (!(l1 = l1->next))
			break;
		mid = &(*mid)->next;
	} while (1);

	l2 = *mid;
	*mid = NULL;

	if ((*flist)->next)
		_mergesort(flist);
	if (l2->next)
		_mergesort(&l2);

	l1 = *flist;

	while (1) {
		if (cmp(l1, l2) <= 0) {
			if (!(l1 = *(flist = &l1->next))) {
				*flist = l2;
				break;
			}
		} else {
			*flist = l2;
			l2 = *(flist = &l2->next);
			*flist = l1;

			if (!l2)
				break;
		}
	}
}

static int
mkcol(struct column *col, struct max *max)
{
	int twidth = 0;

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
newfile(const char *str)
{
	char user[32], group[32];
	struct file *new;
	struct group *gr;
	struct passwd *pw;
	struct stat st;

	if (!(new = malloc(1 * sizeof(*new))))
		goto err;

	new->name  = NULL;
	new->group = NULL;
	new->user  = NULL;

	if (lstat(str, &new->st) < 0)
		goto err;

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
		if (!(stat(str, &st)))
			new->tmode = st.st_mode;
		else
			new->tmode = 0;
	}

	if (!(new->name = strdup(str)))
		goto err;

	new->len  = strlen(str);

	if (!lflag)
		goto done;

	if (!nflag && (pw = getpwuid(new->st.st_uid)))
		snprintf(user, sizeof(user), "%s", pw->pw_name);
	else
		snprintf(user, sizeof(user), "%d", new->st.st_uid);

	if (!nflag && (gr = getgrgid(new->st.st_gid)))
		snprintf(group, sizeof(group), "%s", gr->gr_name);
	else
		snprintf(group, sizeof(group), "%d", new->st.st_gid);

	if (!(new->group = strdup(group)))
		goto err;

	if (!(new->user  = strdup(user)))
		goto err;

	new->glen  = strlen(group);
	new->ulen  = strlen(user);

	goto done;
err:
	err(1, "newfile %s", str);
done:
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
	char type = '\0';

	switch (mode & S_IFMT) {
	case S_IFDIR:
		type = '/';
		break;
	case S_IFIFO:
		type = '|';
		break;
	case S_IFLNK:
		type = '@';
		break;
	case S_IFSOCK:
		type = '=';
		break;
	}

	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		type = '*';

	putchar(type);
	return (type ? 1 : 0);
}

static void
pmode(struct stat *st)
{
	char mode[]  = "?---------";

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
	const char *ch;
	int chcnt = 0, len = 0;
	Rune rune;

	if (iflag && ino)
		chcnt += printf("%*llu ", ino,
			 (unsigned long long)file->st.st_ino);
	if (sflag && size)
		chcnt += printf("%*lld ", size,
			 howmany((long long)file->st.st_blocks, blocksize));

	for (ch = file->name; *ch; ch += len) {
		len = chartorune(&rune, ch);

		if (!qflag || isprintrune(rune))
			chcnt += fwrite(ch, sizeof(char), len, stdout);
		else
			chcnt += fwrite("?", sizeof(char), 1, stdout);
	}

	if (Fpflag == 'F' || (Fpflag == 'p' && S_ISDIR(file->st.st_mode)))
		chcnt += ptype(file->tmode);

	return chcnt;
}

static void
ptime(struct timespec t)
{
	char *fmt, buf[DATELEN];
	struct tm *tm;

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
	char buf[BUFSIZ];
	ssize_t len;
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
			       howmany((long long)p->st.st_blocks, blocksize));

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
			if ((len = readlink(p->name, buf, sizeof(buf) - 1)) < 0)
				err(1, "readlink %s", p->name);
			buf[len] = '\0';

			printf(" -> %s", buf);
			ptype(p->tmode);
		}
next:
		putchar('\n');
	}
}

static void
printc(struct file *flist, struct max *max)
{
	int chcnt = 0, row = 0, i = 0;
	int col, base, num, nrows;
	struct column cols;
	struct file *p, **pa;

	if (mkcol(&cols, max)) {
		print1(flist, max);
		return;
	}

	num = max->total;
	nrows = (num / cols.num);

	if (num % cols.num)
		nrows += 1;

	/* create a array of pointers for random access */
	if (!(pa = malloc(max->total * sizeof(struct file *))))
		err(1, "malloc");

	for (p = flist; p; p = p->next, i++)
		pa[i] = p;

	for (; row < nrows; row++) {
		for (base = row, col = 0; col < cols.width; col++) {
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
	int chcnt = 0, width = 0;
	struct file *p;

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
	int chcnt = 0, col = 0;
	struct column cols;
	struct file *p;

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
printls(struct file **flist, struct max *max)
{
	if (sflag || iflag || lflag)
		printf("total: %lu\n",
		       howmany((long unsigned)max->btotal, blocksize));

	if (!(max->total <= 1) && Sftflag != 'f')
		_mergesort(flist);

	mkmax(max, NULL);
	printfcn(*flist, max);
}

static int
lsdir(const char *path, int more)
{
	DIR *dirp;
	struct dirent *dir;
	struct file *flist = NULL, *p;
	struct max max = {0};

	if (!(dirp = opendir(path))) {
		warn("open_dir %s", path);
		return 1;
	}

	if (more || Rdflag == 'R') {
		printf(first ? "%s:\n" : "\n%s:\n", path);
		first = 0;
	}

	chdir(path);

	while ((dir = readdir(dirp))) {
		if (Aaflag != 'a' && ISDOT(dir->d_name))
			continue;

		if (!Aaflag && dir->d_name[0] == '.')
			continue;

		pushfile(&flist, newfile(dir->d_name));
		mkmax(&max, flist);
	}

	if (max.total)
		printls(&flist, &max);

	if (Rdflag == 'R') {
		for (p = flist; p; p = p->next) {
			if (ISDOT(p->name))
				continue;
			if (!S_ISDIR(p->st.st_mode))
				continue;
			lsdir(pcat(p->name, path, 1), more);
		}
	}

	while (flist)
		freefile(popfile(&flist));

	closedir(dirp);

	return 0;
}

static int
ls(char **argv, int more)
{
	int rval = 0;
	struct file *flist = NULL, *dlist = NULL, *p;
	struct max max = {0};
	struct stat st;

	for (; *argv; argv++) {
		if ((FS_FOLLOW(0) ? stat : lstat)(*argv, &st) < 0) {
			warn("(l)stat %s", *argv);
			rval = 1;
			continue;
		}

		if (Rdflag != 'd' && S_ISDIR(st.st_mode)) {
			pushfile(&dlist, newfile(*argv));
		} else {
			pushfile(&flist, newfile(*argv));
			mkmax(&max, flist);
		}
	}

	if (dlist && dlist->next && Sftflag != 'f')
		_mergesort(&dlist);

	if (max.total) {
		first = 0;
		printls(&flist, &max);
	}

	for (p = dlist; p; p = p->next)
		rval |= lsdir(p->name, more);

	while (flist)
		freefile(popfile(&flist));
	while (dlist)
		freefile(popfile(&dlist));

	return rval;
}


int
main(int argc, char *argv[])
{
	char *temp;
	int kflag = 0;
	struct winsize w;

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
		wrong(usage);
	} ARGEND

	if (!(ioctl(fileno(stdout), TIOCGWINSZ, &w)) && w.ws_col > 0)
		termwidth = w.ws_col;

	if ((printfcn != print1) && lflag)
		lflag = 0;

	if (lflag || sflag) {
		if (!kflag && (temp = getenv("BLOCKSIZE")))
			blocksize = estrtonum(temp, 0, LONG_MAX);
		else if (kflag)
			blocksize = 1024;

		blocksize /= 512;
	}

	if (printfcn != print1 && (temp = getenv("COLUMNS")))
		termwidth = estrtonum(temp, 0, UINT_MAX);

	if (!argc) {
		argv[0] = ".";
		argv[1] = NULL;
	}

	exit(ls(argv, argc > 1));
}
