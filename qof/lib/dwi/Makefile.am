## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libdwi.la

libdwi_la_LDFLAGS= -version-info $(LIBDWI_LIBRARY_VERSION) -release $(VERSION) 

libdwi_la_SOURCES = \
	duifield.c \
	duifieldmap.c \
	duifield-qof.c 

dwiincludedir = ${pkgincludedir}
dwiinclude_HEADERS = \
	duifield.h 

noinst_PROGRAMS = sql_parse_test

EXTRA_DIST =       \
	.cvsignore      \
	README          \

AM_CFLAGS = -g     \
	@CFLAGS@        \
	@GLIB_CFLAGS@

INCLUDES = \
	-I$(inclduedir) 


