## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libdwi.la

# libdwi_la_LDFLAGS= -version-info $(LIBDWI_LIBRARY_VERSION) -release $(VERSION) 

libdwi_la_SOURCES = \
	duifield.c       \
	duifield-qof.c   \
	duifield-sql.c   \
	duifieldmap.c    \
	duifilter.c      \
	duiresolver.c    \
	duitxnquery.c    \
	duitxnreport.c 

dwiincludedir = ${pkgincludedir}
dwiinclude_HEADERS = \
	duifield.h       \
	duifield-qof.h   \
	duifield-sql.h   \
	duifieldmap.h    \
	duifilter.h      \
	duiresolver.h    \
	duitxnquery.h    \
	duitxnreport.h 

EXTRA_DIST =       \
	.cvsignore      \
	README

AM_CFLAGS = -g     \
	@CFLAGS@        \
	@GLIB_CFLAGS@

INCLUDES = \
	-I$(inclduedir) 


