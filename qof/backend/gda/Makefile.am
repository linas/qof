AM_CFLAGS = \
  $(warnFLAGS) \
  -I.. -I../.. \
  -I${top_srcdir}/qof/ \
  -DLOCALE_DIR=\""$(datadir)/locale"\" \
  ${GLIB_CFLAGS}  \
  ${LIBGDA_CFLAGS}

pkglib_LTLIBRARIES=libqof-backend-gda.la

libqof_backend_gda_la_SOURCES = \
 qof-gda.c

libqof_backend_gda_la_LDFLAGS = \
 -L${top_srcdir}/qof \
 -avoid-version -module

#-version-info $(LIBQOF_BACKEND_QGDA_VERSION)

libqof_backend_gda_la_LIBADD = \
 ${QOF_LIBS} \
 ${GLIB_LIBS} \
 ${LIBGDA_LIBS}

EXTRA_DIST = \
 qof-gda.h
