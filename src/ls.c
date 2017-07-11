/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <grp.h>
#include <libgen.h>
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

#define REALLOC(a, b) realloc((a), (b) * (sizeof(*(a))))

typedef struct {
	int width, num;
} LS_COL;

typedef struct {
	char *name, *path, *user, *group;
	size_t len, ulen, glen;
	struct stat info;
	struct timespec tm;
} LS_ENT;

typedef struct {
	int s_block, s_gid, s_nlink;
	int s_ino, s_size, s_uid, len;
	blkcnt_t block;
	ino_t ino;
	nlink_t nlink;
	off_t size;
	size_t btotal, total;
} LS_MAX;

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

static void printc(LS_ENT *, LS_MAX *);
static void (*printfcn)(LS_ENT *, LS_MAX *) = printc;

SET_USAGE = "%s [-1AaCcFfiklmnpqrSstux] [file ...]";

static int
cmp(const void *va, const void *vb)
{
	int cmp;
	const LS_ENT *a = va, *b = vb;

	switch (Sftflag) {
	case 'S':
		cmp = b->info.st_size - a->info.st_size;
		break;
	case 't':
		if (!(cmp = b->tm.tv_sec - a->tm.tv_sec))
			cmp = b->tm.tv_nsec - a->tm.tv_nsec;
		break;
	default:
		cmp = strcmp(a->name, b->name);
		break;
	}

	return (rflag ? (0 - cmp) : cmp);
}

/* make structures */
static int
mkcol(LS_COL *col, LS_MAX *max)
{
	int twidth = 0;

	col->width = max->len;
	if (iflag)
		col->width += max->s_ino + 1;
	if (sflag)
		col->width += max->s_block + 1;
	if (Fpflag)
		col->width += 1;

	col->width += 1;
	twidth = termwidth + 1;

	if (twidth < (2 * col->width))
		return 1;

	col->num   = (twidth / col->width);
	col->width = (twidth / col->num);

	return 0;
}

static void
mkent(LS_ENT *ent, char *path, int cut, struct stat *st)
{
	char user[32], group[32];
	struct group  *gr;
	struct passwd *pw;

	ent->info = *st;
	ent->path = path;
	ent->name = cut ? basename(ent->path) : ent->path;
	ent->len  = strlen(ent->name);

	switch (cuflag) {
	case 'c':
		ent->tm = st->st_ctim;
		break;
	case 'u':
		ent->tm = st->st_atim;
		break;
	default:
		ent->tm = st->st_mtim;
		break;
	}

	if (!lflag)
		return;

	if (!nflag && (pw = getpwuid(st->st_uid)))
		snprintf(user, sizeof(user), "%s", pw->pw_name);
	else
		snprintf(user, sizeof(user), "%d", st->st_uid);

	if (!nflag && (gr = getgrgid(st->st_gid)))
		snprintf(group, sizeof(group), "%s", gr->gr_name);
	else
		snprintf(group, sizeof(group), "%d", st->st_gid);

	if (!(ent->group = strdup(group)))
		perr(1, "strdup:");

	if (!(ent->user = strdup(user)))
		perr(1, "strdup:");

	ent->ulen = strlen(user);
	ent->glen = strlen(group);
}

static void
mkmax(LS_MAX *max, LS_ENT *ent, size_t total)
{
	char buf[21];

	if (total) {
		max->total = total;

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

	max->block = MAX(ent->info.st_blocks, max->block);
	max->ino   = MAX(ent->info.st_ino, max->ino);
	max->len   = MAX(ent->len, max->len);
	max->nlink = MAX(ent->info.st_nlink, max->nlink);
	max->s_gid = MAX(ent->glen, max->s_gid);
	max->s_uid = MAX(ent->ulen, max->s_uid);
	max->size  = MAX(ent->info.st_size, max->size);

	max->btotal += ent->info.st_blocks;
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
pmode(struct stat *st) {
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
pname(LS_ENT *ent, int ino, int size)
{
	const char *ch;
	int chcnt = 0, len = 0;
	Rune rune;

	if (iflag && ino)
		chcnt += printf("%*llu ", ino,
			 (unsigned long long)ent->info.st_ino);
	if (sflag && size)
		chcnt += printf("%*lld ", size,
			 howmany((long long)ent->info.st_blocks, blocksize));

	for (ch = ent->name; *ch; ch += len) {
		len = chartorune(&rune, ch);

		if (!qflag || isprintrune(rune))
			chcnt += fwrite(ch, sizeof(char), len, stdout);
		else
			chcnt += fwrite("?", sizeof(char), 1, stdout);
	}

	if (Fpflag == 'F' || (Fpflag == 'p' && S_ISDIR(ent->info.st_mode)))
		chcnt += ptype(ent->info.st_mode);

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
print1(LS_ENT *ents, LS_MAX *max)
{
	char buf[BUFSIZ];
	LS_ENT *ep;
	ssize_t i, len;
	struct stat *st;

	for (i = 0; i < max->total; i++) {
		ep = &ents[i];
		st = &ents[i].info;

		if (!lflag) {
			pname(ep, max->s_ino, max->s_block);
			goto next;
		}

		if (iflag)
			printf("%*llu ", max->s_ino,
			       (unsigned long long)st->st_ino);
		if (sflag)
			printf("%*lld ", max->s_block,
			       howmany((long long)st->st_blocks, blocksize));

		pmode(st);
		printf("%*lu %-*s %-*s ", max->s_nlink, st->st_nlink,
		       max->s_uid, ep->user, max->s_gid, ep->group);

		if (S_ISBLK(st->st_mode) || S_ISCHR(st->st_mode))
			printf("%3d, %3d ",
			       major(st->st_rdev), minor(st->st_rdev));
		else
			printf("%*s%*lld ", 8 - max->s_size, "",
			       max->s_size, (long long)st->st_size);

		ptime(ep->tm);
		pname(ep, 0, 0);

		if (S_ISLNK(st->st_mode)) {
			if ((len = readlink(ep->path, buf, sizeof(buf) - 1)) < 0)
				perr(1, "readlink %s:", ep->name);
			buf[len] = '\0';

			printf(" -> %s", buf);
			ptype(st->st_mode);
		}
next:
		putchar('\n');
	}
}

static void
printc(LS_ENT *ents, LS_MAX *max)
{
	int chcnt = 0, row = 0;
	int col, base, num, nrows;
	LS_COL cols;

	if (mkcol(&cols, max)) {
		print1(ents, max);
		return;
	}

	num = max->total;
	nrows = (num / cols.num);

	if (num % cols.num)
		nrows += 1;

	for (; row < nrows; row++) {
		for (base = row, col = 0; col < cols.width; col++) {
			chcnt = pname(&ents[base], max->s_ino, max->s_block);
			if ((base += nrows) >= num)
				break;
			while (chcnt++ < cols.width)
				putchar(' ');
		}
		putchar('\n');
	}
}

static void
printm(LS_ENT *ents, LS_MAX *max)
{
	int chcnt = 0, width = 0;
	size_t i = 0;

	if (iflag)
		width += max->s_ino;
	if (sflag)
		width += max->s_block;
	if (Fpflag)
		width += 1;

	for (; i < max->total; i++) {
		if (chcnt > 0) {
			putchar(',');
			if ((chcnt += 3) + width + ents[i].len >= termwidth)
				putchar('\n'), chcnt = 0;
			else
				putchar(' ');
		}

		chcnt += pname(&ents[i], max->s_ino, max->s_block);
	}

	putchar('\n');
}

static void
printx(LS_ENT *ents, LS_MAX *max)
{
	int chcnt = 0, col = 0;
	size_t i = 0;
	LS_COL cols;

	if (mkcol(&cols, max)) {
		print1(ents, max);
		return;
	}

	for (; i < max->total; col++, i++) {
		if (col >= cols.num) {
			col = 0;
			putchar('\n');
		}

		chcnt = pname(&ents[i], max->s_ino, max->s_block);
		while (chcnt++ < cols.width)
			putchar(' ');
	}

	putchar('\n');
}

/* ls functions */
static int
ls_folder(LS_ENT *ent, int more, int depth)
{
	FS_DIR dir;
	size_t i = 0, size = 0;
	LS_ENT *ents = NULL;
	LS_MAX max = {0};

	if (open_dir(&dir, ent->path) < 0)
		return (pwarn("open_dir %s:", ent->path));

	if (Rdflag == 'R' || more) {
		printf(first ? "%s:\n" : "\n%s:\n", ent->path);
		first = 0;
	}

	while (read_dir(&dir, depth) != EOF) {
		if (Aaflag != 'a' && ISDOT(dir.name))
			continue;

		if (!Aaflag && dir.name[0] == '.')
			continue;

		if (!(ents = REALLOC(ents, ++size)))
			perr(1, "realloc:");

		mkent(&ents[size - 1], dir.path, 1, &dir.info);
		mkmax(&max, &ents[size - 1], 0);

		dir.path = NULL; /* Avoid Free*/
	}

	if (sflag || iflag || lflag)
		printf("total: %lu\n",
		       howmany((long unsigned)max.btotal, blocksize));

	if (!size)
		goto jump;

	if (size != 1 && Sftflag != 'f')
		qsort(ents, size, sizeof(*ents), cmp);

	mkmax(&max, NULL, size);
	printfcn(ents, &max);

jump:
	if (Rdflag == 'R') {
		for (i = 0; i < size; i++) {
			if (ISDOT(ents[i].name))
				continue;
			if (!S_ISDIR(ents[i].info.st_mode))
				continue;
			ls_folder(&ents[i], more, depth+1);
		}
	}

	for (i = 0; i < size; i++) {
		free(ents[i].path);

		if (!lflag)
			continue;

		free(ents[i].user);
		free(ents[i].group);
	}

	free(ents);

	return 0;
}

static int
ls(int argc, char **argv)
{
	int ds = 0, fs = 0, i = 0, rval = 0;
	LS_ENT *fents = NULL, *dents = NULL;
	LS_MAX max = {0};
	struct stat st;

	for (; i < argc; i++) {
		if ((FS_FOLLOW(0) ? stat : lstat)(argv[i], &st) < 0) {
			rval = pwarn("(l)stat %s:", argv[i]);
			continue;
		}

		if (Rdflag != 'd' && S_ISDIR(st.st_mode)) {
			if (!(dents = REALLOC(dents, ++ds)))
				perr(1, "realloc:");
			mkent(&dents[ds - 1], argv[i], 0, &st);
		} else {
			if (!(fents = REALLOC(fents, ++fs)))
				perr(1, "realloc:");
			mkent(&fents[fs - 1], argv[i], 0, &st);
			mkmax(&max, &fents[fs - 1], 0);
		}
	}

	if (ds > 1 && Sftflag != 'f')
		qsort(dents, ds, sizeof(*dents), cmp);

	if (fs > 1 && Sftflag != 'f')
		qsort(fents, fs, sizeof(*fents), cmp);

	if (!fs)
		goto printdir;

	first = 0;
	if (sflag || iflag || lflag)
		printf("total %lu\n",
		       howmany((long unsigned)max.btotal, blocksize));

	mkmax(&max, NULL, fs);
	printfcn(fents, &max);

printdir:
	for (i = 0; i < ds; i++)
		ls_folder(&dents[i], argc-1, 0);

	if (lflag) {
		for (i = 0; i < fs; i++) {
			free(fents[i].group);
			free(fents[i].user);
		}
	}

	free(fents);
	free(dents);

	return rval;
}

int
main(int argc, char *argv[])
{
	char *temp;
	int kflag = 0, rval = 0;
	struct winsize w;

	if (!(ioctl(fileno(stdout), TIOCGWINSZ, &w)) && w.ws_col > 0)
		termwidth = w.ws_col;

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

	if (lflag && printfcn != print1)
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

	if (!argc)
		argc++, *argv = ".";

	rval = ls(argc, argv);

	return (rval | fshut("<stdout>", stdout));
}
