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
	src/rmdir\
	src/sync\
	src/true\
	src/tty\
	src/unlink\
	src/yes

# LIB SOURCE
LIBFSSRC=\
	lib/fs/cp.c\
	lib/fs/fst.c

LIBUTILSRC=\
	lib/util/fshut.c\
	lib/util/warn.c

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
clean:
	rm -f $(BIN) $(OBJ) $(LIB)

.PHONY:
	all install clean
