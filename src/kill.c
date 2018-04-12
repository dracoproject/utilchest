#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "util.h"

struct {
	char *str;
	int num;
} sigtab[] = {
	{ "0",    0       },
	{ "ABRT", SIGABRT },
	{ "ALRM", SIGALRM },
	{ "BUS",  SIGBUS  },
	{ "CHLD", SIGCHLD },
	{ "CONT", SIGCONT },
	{ "FPE",  SIGFPE  },
	{ "HUP",  SIGHUP  },
	{ "ILL",  SIGILL  },
	{ "INT",  SIGINT  },
	{ "KILL", SIGKILL },
	{ "PIPE", SIGPIPE },
	{ "QUIT", SIGQUIT },
	{ "SEGV", SIGSEGV },
	{ "STOP", SIGSTOP },
	{ "TERM", SIGTERM },
	{ "TSTP", SIGTSTP },
	{ "TTIN", SIGTTIN },
	{ "TTOU", SIGTTOU },
	{ "USR1", SIGUSR1 },
	{ "USR2", SIGUSR2 },
	{ "URG",  SIGURG  }
};

static int
strtosig(char *str)
{
	int i;

	for (i = 0; i < LEN(sigtab); i++)
		if (!strcasecmp(sigtab[i].str, str))
			return sigtab[i].num;

	errno = EINVAL;
	err(1, "strtosig %s", str);

	/* NOTREACHED */
	return -1;
}

static char *
sigtostr(int sig)
{
	int i;

	for (i = 0; i < LEN(sigtab); i++)
		if (sigtab[i].num == sig)
			return sigtab[i].str;

	errno = EINVAL;
	err(1, "sigtostr %d", sig);

	/* NOTREACHED */
	return NULL;
}

static void
usage(void)
{
	fprintf(stderr,
	        "usage: %s [-s signal_name] pid ...\n"
	        "       %s -l [exit_status]\n",
	        getprogname(), getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	pid_t pid;
	int i, rval, sig;

	rval = 0;
	sig  = SIGTERM;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'l':
		argc--, argv++;
		if (argc > 1)
			usage();
		if (argc) {
			if ((sig = strtobase(*argv, 0, INT_MAX, 10)) >= 128)
				sig -= 128;
			puts(sigtostr(sig));
		} else {
			for (i = 0; i < LEN(sigtab); i++)
				puts(sigtab[i].str);
		}
		exit(ioshut());
	case 's':
		sig = strtosig(EARGF(usage()));
		break;
	default:
		usage();
	} ARGEND

	if (!argc)
		usage();

	for (; *argv; argc--, argv++) {
		pid = strtobase(*argv, INT_MIN, INT_MAX, 10);
		if (kill(pid, sig) < 0) {
			warn("kill %d %d", pid, sig);
			rval = 1;
		}
	}

	return rval;
}
