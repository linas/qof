#
# Makefile.am for the QOF example directory
#

SUBDIRS = gobject

query_example_SOURCES = \
	query-example.c      \
	my-object.c 

sql_example_SOURCES = \
	sql-example.c      \
	my-object.c 

noinst_PROGRAMS = \
	query-example  \
	sql-example

noinst_HEADERS = \
	my-object.h

EXTRA_DIST =     \
	README        \
	.cvsignore


AM_CFLAGS =           \
  -I${top_srcdir}/src \
  ${GLIB_CFLAGS}


LDADD =                                   \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  -lltdl -lm

