#
# Makefile.am for the QOF example directory
#
if BUILD_GTK
EXAMPLES=gobject
else
EXAMPLES=
endif

SUBDIRS = $(EXAMPLES)

# SUBDIRS = gobject

test_book_merge_SOURCES = test-book-merge.c

query_example_SOURCES = \
	query-example.c      \
	my-object.c 

sql_example_SOURCES = \
	sql-example.c      \
	my-object.c 

noinst_PROGRAMS = \
	test-book-merge \
	query-example  \
	sql-example

noinst_HEADERS = \
	my-object.h

EXTRA_DIST =     \
	README        \
	.cvsignore


AM_CFLAGS =           \
  ${GLIB_CFLAGS}


LDADD =                                   \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/qof/qof_book_merge.lo  \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  -lltdl -lm

