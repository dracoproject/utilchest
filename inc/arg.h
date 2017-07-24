/* This file is part of the UtilChest from EltaninOS
 * See LICENSE file for copyright and license details.
 */
#ifndef __ARG_H__
#define __ARG_H__

#define ARGBEGIN \
for (argc--, argv++;\
     *argv && (*argv)[0] == '-' && (*argv)[1]; argc--, argv++) {\
	char _argc, _brk;\
	if ((*argv)[1] == '-' && (*argv)[2] == '\0') {\
		argc -= 1, argv += 1;\
		break;\
	}\
	for (_brk = 0, argv[0]++; (*argv)[0] && !_brk; argv[0]++) {\
		_argc = (*argv)[0];\
		switch (_argc)

#define ARGEND } }

#define ARGC() _argc;

#define ARGF() \
((argv[0][1] == '\0' && !argv[1]) ? (char *)0 :\
(_brk = 1, (argv[0][1] != '\0') ? (&argv[0][1]) : (argc--, argv++, argv[0])))

#define EARGF(x) \
((argv[0][1] == '\0' && !argv[1]) ? ((x), abort(), (char *)0) :\
(_brk = 1, (argv[0][1] != '\0') ? (&argv[0][1]) : (argc--, argv++, argv[0])))

#endif
