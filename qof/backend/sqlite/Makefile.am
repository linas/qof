AM_CFLAGS = \
  -I.. -I../.. \
  -I${top_srcdir}/qof/ \
  -DLOCALE_DIR=\""$(datadir)/locale"\" \
  ${GLIB_CFLAGS}  \
  ${DEPS_CFLAGS} \
  ${SQLITE_CFLAGS}

lib_LTLIBRARIES=libqof-backend-sqlite.la

libqof_backend_sqlite_la_SOURCES = \
 qof-sqlite.c

libqof_backend_sqlite_la_LDFLAGS = \
 -version-info $(LIBQOF_BACKEND_QSQL_VERSION)

libqof_backend_sqlite_la_LIBADD = \
 ${GLIB_LIBS} \
 ${SQLITE_LIBS}

EXTRA_DIST = \
 qof-sqlite.h
