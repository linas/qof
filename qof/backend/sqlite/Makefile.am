AM_CFLAGS = \
  -I.. -I../.. -I../../qof/ \
  -DLOCALE_DIR=\""$(datadir)/locale"\" \
  -DPREFIX=\"@prefix@\" \
  -I${includedir} \
  ${GLIB_CFLAGS}  \
  ${QOF_CFLAGS} \
  ${DEPS_CFLAGS} \
  ${sqlite_CFLAGS}

pkglib_LTLIBRARIES=libqof-backend-sqlite.la

libqof_backend_sqlite_la_SOURCES = \
 qof-sqlite.c

libqof_backend_sqlite_la_LDFLAGS = \
 -version-info $(LIBQOF_BACKEND_QSQL_VERSION)

libqof_backend_sqlite_la_LIBADD = \
 ${GLIB_LIBS} \
 ${sqlite_LIBS}

EXTRA_DIST = \
 qof-sqlite.h
