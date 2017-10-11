include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

INC= inc

HDR=\
	inc/arg.h\
	inc/compat.h\
	inc/utf.h\
	inc/util.h

# SOURCE
BIN=\
	src/basename\
	src/cat\
	src/chgrp\
	src/chmod\
	src/chown\
	src/chroot\
	src/clear\
	src/cp\
	src/date\
	src/dirname\
	src/domainname\
	src/echo\
	src/env\
	src/false\
	src/head\
	src/hostname\
	src/id\
	src/link\
	src/ln\
	src/ls\
	src/mkdir\
	src/mkfifo\
	src/mknod\
	src/mv\
	src/nice\
	src/printenv\
	src/pwd\
	src/readlink\
	src/rm\
	src/rmdir\
	src/sleep\
	src/sync\
	src/true\
	src/tty\
	src/uname\
	src/unlink\
	src/whoami\
	src/yes

# LIB SOURCE
LIBUTFSRC=\
	lib/utf/chartorune.c\
	lib/utf/iscntrlrune.c\
	lib/utf/isprintrune.c\
	lib/utf/isvalidrune.c\
	lib/utf/runetype.c

LIBUTILSRC=\
	lib/util/call.c\
	lib/util/chown.c\
	lib/util/cp.c\
	lib/util/dir.c\
	lib/util/ealloc.c\
	lib/util/mode.c\
	lib/util/stoll.c

# LIB PATH
LIBUTF=  lib/libutf.a
LIBUTIL= lib/libutil.a

# LIB OBJS
LIBUTFOBJ=  $(LIBUTFSRC:.c=.o)
LIBUTILOBJ= $(LIBUTILSRC:.c=.o)

# ALL
LIB= $(LIBUTIL) $(LIBFS) $(LIBUTF)
OBJ= $(BIN:=.o) $(LIBUTILOBJ) $(LIBFSOBJ) $(LIBUTFOBJ)
SRC= $(BIN:=.c)

# VAR RULES
all: $(BIN)

$(BIN): $(LIB) $(@:=.o)
$(OBJ): $(HDR) config.mk

# SUFFIX RULES
.o:
	$(CC) $(LDFLAGS) -o $@ $< $(LIB)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -I $(INC) -o $@ -c $<

# LIBRARIES RULES
$(LIBUTF): $(LIBUTFOBJ)
	$(AR) rc $@ $?
	$(RANLIB) $@

$(LIBUTIL): $(LIBUTILOBJ)
	$(AR) rc $@ $?
	$(RANLIB) $@

# USER ACTIONS
utilchest: $(LIB) $(SRC)
	mkdir -p build
	for f in $(SRC); do sed "s/^main(/$$(echo "$$(basename $${f%.c})" | sed s/-/_/g)_&/" < $$f > build/$$(basename $$f); done
	echo '#include <libgen.h>'                                                                                                                              > build/$@.c
	echo '#include <stdio.h>'                                                                                                                               >> build/$@.c
	echo '#include <string.h>'                                                                                                                              >> build/$@.c
	for f in $(SRC); do echo "int $$(echo "$$(basename $${f%.c})" | sed s/-/_/g)_main(int, char **);"; done                                                 >> build/$@.c
	echo 'int main(int argc, char *argv[]) { char *s = basename(argv[0]);'                                                                                  >> build/$@.c
	echo 'if(!strcmp(s,"utilchest")) { argc--; argv++; s = basename(argv[0]); } if(0) ;'                                                                    >> build/$@.c
	for f in $(SRC); do echo "else if(!strcmp(s, \"$$(basename $${f%.c})\")) return $$(echo "$$(basename $${f%.c})" | sed s/-/_/g)_main(argc, argv);"; done >> build/$@.c
	echo 'else { '                                                                                                                                          >> build/$@.c
	for f in $(SRC); do echo "fputs(\"$$(basename $${f%.c}) \", stdout);"; done                                                                             >> build/$@.c
	echo 'putchar(0xa); }; return 0; }'                                                                                                                     >> build/$@.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -I $(INC) -o $@ build/*.c $(LIB)
	rm -rf build

utilchest-install: utilchest
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	cp -f utilchest $(DESTDIR)/$(PREFIX)/bin
	for f in $$(echo $(BIN) | sed 's/src\///g'); do ln -s utilchest $(DESTDIR)/$(PREFIX)/bin/$$f; done

install: all
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ) $(LIB) utilchest

.PHONY:
	all install clean
