#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <string.h>

int
genpath(char *str, mode_t dmode, mode_t mode)
{
	char *sp, ch;

	ch   = 0;
	sp   = str;

	if ((str[0] == '.' || str[0] == '/') && str[1] == 0)
		return 0;

	for (; *sp; *sp = ch) {
		sp += strspn(sp, "/");
		sp += strcspn(sp, "/");

		ch  = *sp;
		*sp = '\0';

		if (mkdir(str, ch ? dmode : mode) < 0 && errno != EEXIST)
			return -1;
	}

	return 0;
}
