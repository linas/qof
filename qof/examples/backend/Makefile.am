#
# Makefile.am for the QOF example directory
#

my_app_SOURCES = \
	my-instance.c \
	my-app.c

noinst_PROGRAMS =  \
	my-app

noinst_HEADERS = \
	my-instance.h

EXTRA_DIST =       \
	my-instance.sql

AM_CFLAGS =           \
  -I. -I../..        \
  -I${top_srcdir}/qof \
  ${LIBGDA_CFLAGS} \
  ${GLIB_CFLAGS} \
  -Werror

if USE_LIBGDA
LDADD =   \
  ${top_builddir}/qof/libqof.la  \
  ${LIBXML2_LIBS}  \
  ${LIBGDA_LIBS} \
  ${GLIB_LIBS}   \
  ${GMODULE_LIBS} \
  -lltdl -lm
else
LDADD =                                   \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  ${GMODULE_LIBS} \
  -lltdl -lm
endif

