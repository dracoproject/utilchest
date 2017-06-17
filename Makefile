include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

INC= inc

HDR=\
	inc/arg.h\
	inc/fs.h\
	inc/util.h

# SOURCE
BIN=\
	src/basename\
	src/cat\
	src/cp\
	src/dirname\
	src/domainname\
	src/echo\
	src/false\
	src/hostname\
	src/link\
	src/mv\
	src/printenv\
	src/rmdir\
	src/sync\
	src/true\
	src/tty\
	src/uname\
	src/unlink\
	src/whoami\
	src/yes

# LIB SOURCE
LIBFSSRC=\
	lib/fs/cp.c\
	lib/fs/ftr.c

LIBUTILSRC=\
	lib/util/fshut.c\
	lib/util/putstr.c\
	lib/util/mode.c\
	lib/util/warn.c\
	lib/util/wrong.c

# LIB PATH
LIBFS= lib/libfs.a
LIBUTIL= lib/libutil.a

# LIB OBJS
LIBFSOBJ= $(LIBFSSRC:.c=.o)
LIBUTILOBJ= $(LIBUTILSRC:.c=.o)

# ALL
LIB= $(LIBUTIL) $(LIBFS)
OBJ= $(BIN:=.o) $(LIBUTILOBJ) $(LIBFSOBJ)
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
$(LIBFS): $(LIBFSOBJ)
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
