AR = ar
CC = ecc
RANLIB = ranlib

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_FILE_OFFSET_BITS=64
CFLAGS   = -Os -std=c99 -Wall -pedantic
LDFLAGS  =

PREFIX    = /usr/local
MANPREFIX = $(PREFIX)/share/man
