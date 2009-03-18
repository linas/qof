SUBDIRS = . test

lib_LTLIBRARIES = libqof.la

libqof_la_LDFLAGS= -version-info $(LIBQOF_LIBRARY_VERSION) \
  $(LINKER_SCRIPT) -L${top_builddir}/lib/libsql

libqof_la_SOURCES =  \
   qoflog.c \
   guid.c \
   kvpframe.c \
   kvputil.c \
   md5.c \
   qofnumeric.c \
   qofbackend.c \
   qofclass.c \
   qofchoice.c \
   qoferror.c \
   qofevent.c \
   qofgobj.c \
   qofid.c \
   qofinstance.c \
   qofquery.c \
   qofbook.c \
   qofobject.c \
   qofquerycore.c \
   qofreference.c \
   qofsession.c \
   qofsql.c \
   qofutil.c \
   qofbookmerge.c \
   qofdate.c \
   qoftime.c \
   qofstrftime.c \
   qofstrptime.c \
   qofundo.c

#   deprecated.c

qofincludedir = ${pkgincludedir}

qofinclude_HEADERS = \
   qoflog.h \
   guid.h \
   kvpframe.h \
   kvputil.h  \
   qof.h \
   qofbackend.h \
   qofbackend-p.h \
   qofclass.h  \
   qofchoice.h \
   qoferror.h \
   qofevent.h \
   qofgobj.h \
   qofid.h \
   qofid-p.h \
   qofinstance-p.h \
   qofinstance.h \
   qofnumeric.h \
   qofquery.h \
   qofquery-p.h \
   qofbook.h  \
   qofobject.h \
   qofquerycore.h \
   qofquerycore-p.h \
   qofreference.h \
   qofsession.h \
   qofsql.h \
   qofutil.h \
   qofbookmerge.h \
   qofdate.h \
   qoftime.h \
   qofundo.h

noinst_HEADERS = \
   kvputil-p.h \
   md5.h  \
   qofclass-p.h  \
   qofdate-p.h \
   qoferror-p.h \
   qofevent-p.h \
   qofmath128.h  \
   qofquery-deserial.h \
   qofquery-serialize.h \
   qofbook-p.h  \
   qofdate-p.h \
   qofobject-p.h  \
   qofsession-p.h \
   qofundo-p.h \
   deprecated.h \
   qofsql-p.h

AM_CFLAGS = -g \
  -DQOF_LIB_DIR=\"${QOF_LIB_DIR}\" \
  ${warnFLAGS} \
  ${LIBGDA_CFLAGS} \
  ${GLIB_CFLAGS}

if USE_LIBGDA
INCLUDES = \
  -DPREFIX=\""$(prefix)"\"  \
  -DDATADIR=\""$(datadir)"\"
else
INCLUDES = \
  -I${top_srcdir}/lib/libsql  \
  -DPREFIX=\""$(prefix)"\" \
  -DDATADIR=\""$(datadir)"\"
endif

libqof_la_LIBADD = \
  ${GMODULE_LIBS} \
  ${GOBJECT_LIBS} \
  ${GLIB_LIBS} \
  ${GDA_PKG_LIB} \
  ${SQL_PKG_LIB} \
  -lm

EXTRA_DIST = \
  libqof.ver \
  qofmath128.c
