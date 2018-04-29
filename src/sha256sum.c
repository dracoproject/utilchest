#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "crypto.h"
#include "util.h"

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-c] [file ...]\n", getprogname());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct crypto p;
	union hash_state md;
	FILE *fp;
	int (*fn)(struct crypto *, FILE *, const char *);
	int rval;
	uint8_t buf[32];

	fn   = crypto_print;
	rval = 0;
	setprogname(argv[0]);

	ARGBEGIN {
	case 'c':
		fn = crypto_check;
		break;
	default:
		usage();
	} ARGEND

	p = (struct crypto){
		.md       = &md,
		.init     = sha256_init,
		.process  = sha256_process,
		.done     = sha256_done,
		.buf      = buf,
		.bsiz     = sizeof(buf),
	};

	if (!argc)
		fn(&p, stdin, "<stdin>");

	for (; *argv; argc--, argv++) {
		if (ISDASH(*argv)) {
			fp    = stdin;
			*argv = "<stdin>";
		} else if (!(fp = fopen(*argv, "r"))) {
			warn("fopen %s", *argv);
			rval = 1;
			continue;
		}
		rval |= fn(&p, fp, *argv);
		fclose(fp);
	}

	return (ioshut());
}
