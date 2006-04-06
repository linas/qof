#
# Makefile.am for the libqof sources
#

SUBDIRS = . test

lib_LTLIBRARIES = libqof.la

libqof_la_LDFLAGS= -version-info $(LIBQOF_LIBRARY_VERSION) 

libqof_la_SOURCES =  \
   gnc-date.c        \
   gnc-numeric.c     \
   qoflog.c         \
   guid.c            \
   kvp_frame.c       \
   kvp-util.c        \
   md5.c             \
   qofbackend.c      \
   qofclass.c        \
   qofchoice.c       \
   qofevent.c        \
   qofgobj.c         \
   qofid.c           \
   qofinstance.c     \
   qofquery.c        \
   qofbook.c         \
   qofobject.c       \
   qofquerycore.c    \
   qofreference.c    \
   qofsession.c      \
   qofsql.c          \
   qofutil.c         \
   qofbookmerge.c  \
   deprecated.c

qofincludedir = ${pkgincludedir}

qofinclude_HEADERS = \
   gnc-date.h        \
   gnc-numeric.h     \
   qoflog.h \
   guid.h            \
   kvp_frame.h       \
   kvp-util.h        \
   kvp-util-p.h      \
   qof.h             \
   qofbackend.h      \
   qofbackend-p.h    \
   qofclass.h        \
   qofchoice.h       \
   qofevent.h \
   qofgobj.h         \
   qofid.h           \
   qofid-p.h         \
   qofinstance-p.h   \
   qofinstance.h     \
   qofquery.h        \
   qofquery-p.h \
   qofbook.h         \
   qofobject.h       \
   qofquerycore.h    \
   qofquerycore-p.h \
   qofreference.h \
   qofsession.h      \
   qofla-dir.h \
   qofsql.h	     \
   qofutil.h \
   qofbookmerge.h \
   deprecated.h

noinst_HEADERS =      \
   md5.h              \
   qofclass-p.h       \
   qofevent-p.h \
   qofmath128.h  \
   qofquery-deserial.h     \
   qofquery-serialize.h    \
   qofbook-p.h        \
   qofobject-p.h      \
   qofsession-p.h

AM_CFLAGS = -g        \
  ${warnFLAGS} \
	${LIBXML2_CFLAGS}  \
  ${LIBGDA_CFLAGS} \
  ${GLIB_CFLAGS} 

if USE_LIBGDA
INCLUDES = \
  -DPREFIX=\""$(prefix)"\"  \
  -DDATADIR=\""$(datadir)"\"
else
INCLUDES =                     \
   -I${top_srcdir}/lib/libsql             \
   -DPREFIX=\""$(prefix)"\"    \
   -DDATADIR=\""$(datadir)"\" 
endif

libqof_la_LIBADD = \
  ${LIBGDA_LIBS} \
  ${GMODULE_LIBS} \
  ${GOBJECT_LIBS} \
  ${GLIB_LIBS} \
  -lm

EXTRA_DIST = \
  qofla-dir.h.in \
  qofmath128.c \
  deprecated.h
