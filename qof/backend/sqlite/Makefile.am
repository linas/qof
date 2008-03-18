AM_CFLAGS = \
  ${warnFLAGS} \
  -I.. -I../.. \
  -I${top_srcdir}/qof/ \
  -DLOCALE_DIR=\""$(datadir)/locale"\" \
  ${GLIB_CFLAGS}  \
  ${DEPS_CFLAGS} \
  ${SQLITE_CFLAGS}

pkglib_LTLIBRARIES=libqof-backend-sqlite.la

libqof_backend_sqlite_la_SOURCES = \
 qof-sqlite.c

libqof_backend_sqlite_la_LDFLAGS = \
 -L${top_srcdir}/qof \
 -L${top_srcdir}/lib/libsql \
 -avoid-version -module

# -version-info $(LIBQOF_BACKEND_QSQL_VERSION)

libqof_backend_sqlite_la_LIBADD = \
 ${QOF_LIBS} \
 ${GLIB_LIBS} \
 ${SQLITE_LIBS}

EXTRA_DIST = \
 qof-sqlite.h
