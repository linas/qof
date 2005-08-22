#
# Makefile.am for the QOF example directory
#
if BUILD_GTK
EXAMPLES=gobject
else
EXAMPLES=
endif

SUBDIRS = $(EXAMPLES) backend

# SUBDIRS = gobject

CFLAGS += -Werror

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
	README

AM_CFLAGS =  \
  -I.. -I../.. -I../../.. \
  -I${top_srcdir}/qof \
  -I${top_srcdir}/backend/file \
  ${LIBXML2_CFLAGS} \
  ${LIBGDA_CFLAGS} \
  ${GLIB_CFLAGS}

if USE_LIBGDA
test_qsf_LDADD = \
  ${top_builddir}/backend/file/libqof-backend-qsf.la \
  ${top_builddir}/qof/libqof.la           \
  ${LIBXML2_LIBS} \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}   \
  -lltdl
else
test_qsf_LDADD = \
  ${top_builddir}/backend/file/libqof-backend-qsf.la \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}                            \
  -lltdl
endif

if USE_LIBGDA
test_book_merge_LDADD = \
  ${top_builddir}/qof/qof_book_merge.lo    \
  ${top_builddir}/backend/file/libqof-backend-qsf.la \
  ${top_builddir}/qof/libqof.la           \
  ${LIBXML2_LIBS} \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}   \
  -lltdl
else
test_book_merge_LDADD = \
  ${top_builddir}/qof/qof_book_merge.lo    \
  ${top_builddir}/backend/file/libqof-backend-qsf.la \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}                            \
  -lltdl
endif

if USE_LIBGDA
LDADD =   \
  ${top_builddir}/qof/libqof.la \
  ${LIBXML2_LIBS} \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}   \
  -lltdl -lm
else
LDADD =                                   \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}                            \
  -lltdl -lm
endif

