/* This file is part of the UtilChest from Draco Project
 * See LICENSE file for copyright and license details.
 */
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>

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

#define BIG(a, b) (a > b) ? a : b
#define REALLOC(a, b) realloc((a), (b) * (sizeof(*(a))))

struct entry {
	char *name, *path, *user, *group;
	size_t len, ulen, glen;
	struct stat info;
	struct timespec tm;
};

struct esmax {
	int s_block, s_gid, s_nlink;
	int s_ino, s_size, s_uid, len;
	blkcnt_t block;
	ino_t ino;
	nlink_t nlink;
	off_t size;
	size_t btotal, total;
};

struct columns {
	int colwidth, numcols;
};

static int iflag = 0;
static int nflag = 0;
static int qflag = 0;
static int rflag = 0;
static int sflag = 0;

static int first = 1;
static int indicator = 0;
static int recurse = 0;
static int seedot = 0;
static int sort = 0;
static int times = 0;

static int termwidth = 80;
static long blocksize = 512;

static void printacol(struct entry *, struct esmax *);
static void printcol(struct entry *, struct esmax *);
static void printlong(struct entry *, struct esmax *);
static void printscol(struct entry *, struct esmax *);
static void printstream(struct entry *, struct esmax *);

static void (*printfcn)(struct entry *, struct esmax *) = printcol;

SET_USAGE = "%s [-1AaCcFfiklmnpqrSstux] [file ...]";


/* MAKE STRUCTURES */
static int
mkcolumn(struct columns *sco, struct entry *ents, struct esmax *max) {
	int twidth = 0;

	sco->colwidth = max->len;
	if (iflag)
		sco->colwidth += max->s_ino + 1;
	if (sflag)
		sco->colwidth += max->s_block + 1;
	if (indicator)
		sco->colwidth += 1;

	sco->colwidth += 1;
	twidth = termwidth + 1;

	if ((twidth < 2) * sco->colwidth) {
		printscol(ents, max);
		return 1;
	}

	sco->numcols  = (twidth / sco->colwidth);
	sco->colwidth = (twidth / sco->numcols);

	return 0;
}

static void
mkentry(struct entry *ent, char *path, int bname, struct stat *info) {
	char user[32], group[32];
	struct group  *gr;
	struct passwd *pw;

	ent->info = *info;
	ent->path = path;
	ent->name = bname ? basename(ent->path) : ent->path;
	ent->len  = strlen(ent->name);

	switch (times) {
	case 'c':
		ent->tm = info->st_ctim;
		break;
	case 'u':
		ent->tm = info->st_atim;
		break;
	default:
		ent->tm = info->st_mtim;
		break;
	}

	if (printfcn != printlong)
		return;

	if (!nflag && (pw = getpwuid(info->st_uid)))
		snprintf(user, sizeof(user), "%s", pw->pw_name);
	else
		snprintf(user, sizeof(user), "%d", info->st_uid);

	if (!nflag && (gr = getgrgid(info->st_gid)))
		snprintf(group, sizeof(group), "%s", gr->gr_name);
	else
		snprintf(group, sizeof(group), "%d", info->st_gid);

	if (!(ent->group = strdup(group)))
		perr(1, "strdup:");

	if (!(ent->user = strdup(user)))
		perr(1, "strdup:");

	ent->ulen = strlen(user);
	ent->glen = strlen(group);
}

static void
mkesmax(struct esmax *max, struct entry *ent, size_t total) {
	char buf[21];

	if (total)
		goto make;

	/* default */
	max->len   = BIG(ent->len, max->len);
	max->ino   = BIG(ent->info.st_ino, max->ino);
	max->block = BIG(ent->info.st_blocks, max->block);

	/* long */
	max->s_uid = BIG(ent->ulen, max->s_uid);
	max->s_gid = BIG(ent->glen, max->s_gid);
	max->nlink = BIG(ent->info.st_nlink, max->nlink);
	max->size  = BIG(ent->info.st_size, max->size);

	max->btotal += ent->info.st_blocks;

	return;

make:
	max->total = total;

	max->s_block =
	snprintf(buf, sizeof(buf), "%llu", (unsigned long long)max->block);

	max->s_ino =
	snprintf(buf, sizeof(buf), "%llu", (unsigned long long)max->ino);

	max->s_nlink =
	snprintf(buf, sizeof(buf), "%lu", (unsigned long)max->nlink);

	max->s_size =
	snprintf(buf, sizeof(buf), "%lld", (long long)max->size);
}

/* PRINT */
static int
printtype(mode_t mode) {
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
printmode(struct stat *p) {
	char mode[]  = "?---------";

	switch (p->st_mode & S_IFMT) {
	case S_IFBLK: mode[0] = 'b'; break;
	case S_IFCHR: mode[0] = 'c'; break;
	case S_IFDIR: mode[0] = 'd'; break;
	case S_IFIFO: mode[0] = 'p'; break;
	case S_IFLNK: mode[0] = 'l'; break;
	case S_IFREG: mode[0] = '-'; break;
	}

	/* usr */
	if (p->st_mode & S_IRUSR) mode[1] = 'r';
	if (p->st_mode & S_IWUSR) mode[2] = 'w';
	if (p->st_mode & S_IXUSR) mode[3] = 'x';

	if (p->st_mode & S_ISUID)
		mode[3] = (mode[3] == 'x') ? 's' : 'S';

	/* group */
	if (p->st_mode & S_IRGRP) mode[4] = 'r';
	if (p->st_mode & S_IWGRP) mode[5] = 'w';
	if (p->st_mode & S_IXGRP) mode[6] = 'x';

	if (p->st_mode & S_ISGID)
		mode[6] = (mode[6] == 'x') ? 's' : 'S';

	/* other */
	if (p->st_mode & S_IROTH) mode[7] = 'r';
	if (p->st_mode & S_IWOTH) mode[8] = 'w';
	if (p->st_mode & S_IXOTH) mode[9] = 'x';

	if (p->st_mode & S_ISVTX)
		mode[9] = (mode[9] == 'x') ? 't' : 'T';

	printf("%s ", mode);
}

static int
printname(struct entry *ent, int inofield, int sizefield) {
	const char *ch;
	int chcnt = 0, len = 0;
	Rune rune;

	if (iflag && inofield)
		chcnt += printf("%*llu ", inofield,
		         (unsigned long long)ent->info.st_ino);

	if (sflag && sizefield)
		chcnt += printf("%*lld ", sizefield,
		         howmany((long long)ent->info.st_blocks, blocksize));

	for (ch = ent->name; *ch; ch += len) {
		len = chartorune(&rune, ch);
		if (!qflag || isprintrune(rune))
			chcnt += fwrite(ch, sizeof(char), len, stdout);
		else
			chcnt += fwrite("?", sizeof(char), 1, stdout);
	}

	if (indicator == 'F' || (indicator == 'p' && S_ISDIR(ent->info.st_mode)))
		chcnt += printtype(ent->info.st_mode);

	return chcnt;
}

static void
printtime(struct timespec t) {
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

/* PRINT FUNCTIONS */
static void
printacol(struct entry *ents, struct esmax *max) {
	int chcnt = 0, col = 0;
	size_t i = 0;
	struct columns sco;

	if ((mkcolumn(&sco, ents, max)))
		return;

	for (; i < max->total; col++, i++) {
		if (col >= sco.numcols) {
			col = 0;
			putchar('\n');
		}

		chcnt = printname(&ents[i], max->s_ino, max->s_block);
		while (chcnt++ < sco.colwidth)
			putchar(' ');
	}

	putchar('\n');
}

static void
printcol(struct entry *ents, struct esmax *max) {
	int chcnt = 0, col = 0, row = 0;
	int base, num, numrows;
	struct columns sco;

	if ((mkcolumn(&sco, ents, max)))
		return;

	num = max->total;
	numrows = (num / sco.numcols);
	if (num % sco.numcols)
		numrows += 1;

	for (; row < numrows; row++) {
		for (base = row, col = 0; col < sco.colwidth; col++) {
			chcnt = printname(&ents[base], max->s_ino, max->s_block);
			if ((base += numrows) >= num)
				break;
			while (chcnt++ < sco.colwidth)
				putchar(' ');
		}
		putchar('\n');
	}
}

static void
printlong(struct entry *ents, struct esmax *max) {
	char buf[BUFSIZ];
	size_t i = 0;
	ssize_t len;
	struct entry *ep;
	struct stat *p;

	for (; i < max->total; i++) {
		ep = &ents[i];
		p = &ents[i].info;

		if (iflag)
			printf("%*llu ", max->s_ino,
			       (unsigned long long)p->st_ino);
		if (sflag)
			printf("%*lld ", max->s_block,
			       howmany((long long)p->st_blocks, blocksize));

		printmode(p);
		printf("%*ld %-*s %-*s ", max->s_nlink,
		       p->st_nlink, max->s_uid, ep->user, max->s_gid, ep->group);

		if (S_ISBLK(p->st_mode) || S_ISCHR(p->st_mode))
			printf("%3d, %3d ",
			       major(p->st_rdev), minor(p->st_rdev));
		else
			printf("%*s%*lld ", 8 - max->s_size, "",
			       max->s_size, (long long)p->st_size);

		printtime(ep->tm);
		printname(ep, 0, 0);

		if (S_ISLNK(p->st_mode)) {
			if ((len = readlink(ep->path, buf, sizeof(buf) - 1)) < 0)
				perr(1, "readlink %s:", ep->name);

			buf[len] = '\0';
			printf(" -> %s", buf);
			printtype(p->st_mode);
		}

		putchar('\n');
	}
}

static void
printscol(struct entry *ents, struct esmax *max) {
	size_t i = 0;

	for (; i < max->total; i++) {
		printname(&ents[i], max->s_ino, max->s_block);
		putchar('\n');
	}
}

static void
printstream(struct entry *ents, struct esmax *max) {
	int chcnt = 0, width = 0;
	size_t i = 0;

	if (iflag)
		width += max->s_ino;
	if (sflag)
		width += max->s_block;
	if (indicator)
		width += 1;

	for (; i < max->total; i++) {
		if (chcnt > 0) {
			putchar(','), chcnt += 3;
			if (chcnt + width + ents[i].len >= termwidth)
				putchar('\n'), chcnt = 0;
			else
				putchar(' ');
		}
		chcnt += printname(&ents[i], max->s_ino, max->s_block);
	}
	putchar('\n');
}

/* MAIN FUNCTIONS */
static int
cmp(const void *va, const void *vb) {
	int cmp;
	const struct entry *a = va, *b = vb;

	switch (sort) {
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

static int
ls_folder(const char *s, int depth, int more) {
	FS_DIR dir;
	size_t i = 0, size = 0;
	struct entry *ents = NULL;
	struct esmax max = {0}, *pmax = &max;

	if (open_dir(s, &dir) < 0)
		return (pwarn("ls_folder %s:", s));

	if ((recurse == 'R') || more) {
		printf(first ? "%s:\n" : "\n%s:\n", s);
		first = 0;
	}

	while (read_dir(&dir, depth) != EOF) {
		if (seedot != 'a' && ISDOT(dir.name))
			continue;

		if (seedot != 'a' && dir.name[0] == '.')
			continue;

		if (!(ents = REALLOC(ents, ++size)))
			perr(1, "realloc:");

		mkentry(&ents[size - 1], dir.path, 1, &dir.info);
		mkesmax(pmax, &ents[size - 1], 0);

		dir.path = NULL; /* Avoid free */
	}

	if (!size)
		goto printed;

	if ((size != 1) && (sort != 'f'))
		qsort(ents, size, sizeof(*ents), cmp);

	if (sflag || iflag || (printfcn == printlong))
		printf("total %lu\n",
		       howmany((long unsigned)pmax->btotal, blocksize));

	mkesmax(pmax, NULL, size);
	printfcn(ents, pmax);

printed:
	if (recurse == 'R')
		for (i = 0; i < size; i++) {
			if (ISDOT(ents[i].name))
				continue;
			if (!S_ISDIR(ents[i].info.st_mode))
				continue;
			ls_folder(ents[i].path, depth+1, more);
		}

	for (i = 0; i < size; i++) {
		free(ents[i].path);

		if (printfcn != printlong)
			continue;

		free(ents[i].user);
		free(ents[i].group);
	}

	free(ents);

	return 0;
}

static int
ls(int argc, char **argv) {
	char **dents = NULL;
	int fs = 0, ds = 0, i = 0, more = 0, rval = 0;
	struct entry *fents = NULL;
	struct esmax max = {0}, *pmax = &max;
	struct stat st;

	for (; i < argc; i++) {
		if ((FS_FOLLOW(0) ? stat : lstat)(argv[i], &st) < 0) {
			rval = pwarn("(l)stat %s:", argv[i]);
			continue;
		}

		if (recurse != 'd' && S_ISDIR(st.st_mode)) {
			if (!(dents = realloc(dents, ++ds * sizeof(char*))))
				perr(1, "realloc:");

			dents[ds - 1] = argv[i];
			continue;
		}

		if (!(fents = REALLOC(fents, ++fs)))
			perr(1, "realloc:");

		mkentry(&fents[fs - 1], argv[i], 0, &st);
		mkesmax(pmax, &fents[fs - 1], 0);
	}

	if (!fs)
		goto printdir;

	first = 0;
	if ((fs != 1) && (sort != 'f'))
		qsort(fents, fs, sizeof(*fents), cmp);

	if (sflag || iflag || (printfcn == printlong))
		printf("total %lu\n",
		       howmany((long unsigned)max.btotal, blocksize));

	mkesmax(pmax, NULL, fs);
	printfcn(fents, pmax);

	if (printfcn == printlong)
		for (i = 0; i < fs; i++) {
			free(fents[i].user);
			free(fents[i].group);
		}

	free(fents);

printdir:
	more = argc > 1;
	for (i = 0; i < ds; i++)
		ls_folder(dents[i], 0, more);

	free(dents);

	return rval;
}

int
main(int argc, char *argv[]) {
	char *temp;
	int kflag = 0, rval = 0;
	struct winsize w;

	if (!(ioctl(fileno(stdout), TIOCGWINSZ, &w)) && w.ws_col > 0)
		termwidth = w.ws_col;

	ARGBEGIN {
	case 'n':
		nflag = 1;
		/* fallthrough */
	case 'l':
		printfcn = printlong;
		break;
	case 'i':
		iflag = 1;
		break;
	case 'k':
		kflag = 1, blocksize = 1024;
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
	case 'A':
	case 'a':
		seedot = ARGC();
		break;
	case 'C':
		printfcn = printcol;
		break;
	case 'm':
		printfcn = printstream;
		break;
	case 'x':
		printfcn = printacol;
		break;
	case '1':
		printfcn = printscol;
		break;
	case 'F':
	case 'p':
		indicator = ARGC();
		break;
	case 'H':
	case 'L':
		fs_follow = ARGC();
		break;
	case 'R':
	case 'd':
		recurse = ARGC();
		break;
	case 'f':
		rflag = 0;
		seedot = 'a';
		/* fallthrough */
	case 'S':
	case 't':
		sort = ARGC();
		break;
	case 'c':
	case 'u':
		times = ARGC();
		break;
	default:
		wrong(usage);
	} ARGEND

	if ((printfcn == printlong) || sflag) {
		if (!kflag && (temp = getenv("BLOCKSIZE")))
			blocksize = estrtonum(temp, 0, UINT_MAX);
		blocksize /= 512;
	}

	if ((printfcn != printscol) && (temp = getenv("COLUMNS")))
		termwidth = estrtonum(temp, 0, UINT_MAX);

	if (!argc)
		argc++, *argv = ".";

	rval = ls(argc, argv);

	return (rval | fshut("<stdout>", stdout));
}
