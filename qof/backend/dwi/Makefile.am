## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libqof_backend_dwi.la

# libqof_backend_dwi_la_LDFLAGS= -version-info $(LIBDWI_LIBRARY_VERSION) -release $(VERSION) 

libqof_backend_dwi_la_SOURCES = \
	qofmap.c

qof_backend_dwiincludedir = ${pkgincludedir}
qof_backend_dwiinclude_HEADERS = \
	qofmap.h

EXTRA_DIST =       \
	.cvsignore      \
	README

AM_CFLAGS = -g     \
	@CFLAGS@        \
	@GLIB_CFLAGS@

INCLUDES = \
	-I../../lib/dwi


