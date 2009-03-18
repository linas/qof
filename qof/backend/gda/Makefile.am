AM_CFLAGS = \
  ${warnFLAGS} \
  -I.. -I../.. \
  -I${top_srcdir}/qof/ \
  -DLOCALE_DIR=\""$(datadir)/locale"\" \
  ${GLIB_CFLAGS} \
  ${GOBJECT_CFLAGS} \
  ${LIBGDA_CFLAGS}

qof_LTLIBRARIES=libqof-backend-gda.la

qofdir=$(libdir)/qof${SONAME}

libqof_backend_gda_la_SOURCES = \
 qof-gda.c

libqof_backend_gda_la_LDFLAGS = \
 -L${top_srcdir}/qof \
 -avoid-version -module

libqof_backend_gda_la_LIBADD = \
 ${QOF_LIBS} \
 ${GLIB_LIBS} \
 ${GOBJECT_LIBS} \
 ${LIBGDA_LIBS}

EXTRA_DIST = \
 qof-gda.h
