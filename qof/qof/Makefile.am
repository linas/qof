#
# Makefile.am for the libqof sources
#

lib_LTLIBRARIES = libqof.la

libqof_la_LDFLAGS= -version-info $(LIBQOF_LIBRARY_VERSION) 

libqof_la_SOURCES =  \
   gnc-date.c        \
   gnc-engine-util.c \
   gnc-numeric.c     \
   gnc-event.c       \
   gnc-trace.c       \
   guid.c            \
   kvp_frame.c       \
   kvp-util.c        \
   md5.c             \
   qofbackend.c      \
   qofclass.c        \
   qofgobj.c         \
   qofid.c           \
   qofinstance.c     \
   qofquery.c        \
   qofbook.c         \
   qofobject.c       \
   qofquerycore.c    \
   qofsession.c      \
   qofsql.c          \
   qof_book_merge.c

qofincludedir = ${pkgincludedir}

qofinclude_HEADERS = \
   gnc-date.h        \
   gnc-engine-util.h \
   gnc-numeric.h     \
   gnc-event.h       \
   gnc-trace.h       \
   guid.h            \
   kvp_frame.h       \
   kvp-util.h        \
   qof.h             \
   qof-be-utils.h    \
   qofbackend.h      \
   qofclass.h        \
   qofgobj.h         \
   qofid.h           \
   qofid-p.h         \
   qofinstance-p.h   \
   qofinstance.h     \
   qofquery.h        \
   qofbook.h         \
   qofmath128.c      \
   qofmath128.h      \
   qofobject.h       \
   qofquerycore.h    \
   qofsession.h      \
   qofsql.h	     \
   qof_book_merge.h

noinst_HEADERS =      \
   gnc-event-p.h      \
   kvp-util-p.h       \
   md5.h              \
   qofbackend-p.h     \
   qofclass-p.h       \
   qofquery-p.h       \
   qofquery-deserial.h     \
   qofquery-serialize.h    \
   qofbook-p.h        \
   qofobject-p.h      \
   qofquerycore-p.h   \
   qofsession-p.h

EXTRA_DIST =

AM_CFLAGS = -g        \
   @CFLAGS@           \
	${LIBXML2_CFLAGS}  \
   ${GLIB_CFLAGS}

INCLUDES =                     \
   -I${top_srcdir}/lib/libsql             \
   -I$(includedir)             \
   -DPREFIX=\""$(prefix)"\"    \
   -DDATADIR=\""$(datadir)"\" 

LDADD = \
   ${GLIB_LIBS}

