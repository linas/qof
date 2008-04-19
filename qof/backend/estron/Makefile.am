pkglib_LTLIBRARIES = libqof_backend_dwi.la

libqof_backend_dwi_la_LDFLAGS= \
 -L${top_srcdir}/qof \
 -L${top_srcdir}/lib/libsql \
 -avoid-version -module

libqof_backend_dwi_la_LIBADD= \
  $(ESTRON_LIBS) \
  ${QOF_LIBS} \
  ${GLIB_LIBS}

libqof_backend_dwi_la_SOURCES = \
 dwi-backend.c \
 qofmap.c 

qof_backend_dwiincludedir = ${pkgincludedir}
qof_backend_dwiinclude_HEADERS = \
 qofmap.h

EXTRA_DIST = \
 README

AM_CFLAGS =  \
 -I.. -I../.. \
 -I${top_srcdir}/qof/ \
 -DLOCALE_DIR=\""$(datadir)/locale"\" \
 ${GLIB_CFLAGS} \
 ${ESTRON_CFLAGS}
