AM_CFLAGS = \
  -I.. -I../.. -I../../.. \
  -I${top_srcdir}/qof \
  -I${top_srcdir}/backend/file \
   ${LIBXML2_CFLAGS} \
   ${LIBGDA_CFLAGS} \
   ${GLIB_CFLAGS} \
  -Werror \
  -Wmissing-prototypes \
  -Wmissing-declarations

INCLUDES = \
   -I. -I../ -I${top_builddir}/qof \
   -I${top_srcdir}/lib/libsql

if USE_LIBGDA
LDADD = \
   ${top_builddir}/qof/libqof.la \
   ${LIBXML2_LIBS} \
   ${GLIB_LIBS} \
   ${GMODULE_LIBS} \
   ${LIBGDA_LIBS} \
   -lltdl \
   -lm
else
LDADD = \
   ${top_builddir}/qof/libqof.la \
   ${top_builddir}/lib/libsql/libqofsql.la \
   ${LIBXML2_LIBS} \
   ${GLIB_LIBS} \
   ${GMODULE_LIBS} \
   -lltdl \
   -lm
endif

test_book_merge_SOURCES = \
  test-stuff.c \
  test-book-merge.c

test_date_SOURCES = \
  test-date.c \
  test-stuff.c \
  test-engine-stuff.c

test_guid_SOURCES = \
  test-stuff.c \
  test-guid.c

test_numeric_SOURCES = \
  test-stuff.c \
  test-numeric.c

test_object_SOURCES = \
  test-stuff.c \
  test-object.c

test_querynew_SOURCES = \
  test-stuff.c \
  test-querynew.c

noinst_PROGRAMS = \
  test-book-merge \
  test-date \
  test-guid \
  test-numeric \
  test-object \
  test-querynew

TESTS = \
  test-book-merge \
  test-date \
  test-guid \
  test-numeric \
  test-object \
  test-querynew

EXTRA_DIST = \
  test-stuff.h \
  test-engine-stuff.h

