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

test_qsf_SOURCES = test-qsf.c


noinst_PROGRAMS = \
	test-book-merge \
	query-example  \
	sql-example \
	test-qsf

noinst_HEADERS = \
	my-object.h

EXTRA_DIST =     \
	README        \
	.cvsignore


AM_CFLAGS =  \
  -I.. -I../.. \
  -I${top_srcdir}/qof \
  -I${top_srcdir}/backend/file \
  ${LIBXML2_CFLAGS} \
  ${GLIB_CFLAGS}

test_qsf_LDADD = \
  ${top_srcdir}/backend/file/libqof-backend-qsf.la \
  ${top_srcdir}/qof/libqof.la           \
  ${top_srcdir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  -lltdl

#test_book_merge_LDADD = \
#  ${top_builddir}/qof/qof_book_merge.lo


LDADD =                                   \
  ${top_srcdir}/qof/libqof.la           \
  ${top_srcdir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  -lltdl -lm

