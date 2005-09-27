#
# Makefile.am for the libqof sources
#

SUBDIRS = . test

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
   qofchoice.c       \
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
   kvp-util-p.h      \
   qof.h             \
   qof-be-utils.h    \
   qofbackend.h      \
   qofbackend-p.h    \
   qofclass.h        \
   qofchoice.h       \
   qofgobj.h         \
   qofid.h           \
   qofid-p.h         \
   qofinstance-p.h   \
   qofinstance.h     \
   qofquery.h        \
   qofbook.h         \
   qofobject.h       \
   qofquerycore.h    \
   qofsession.h      \
   qofsql.h	     \
   qof_book_merge.h

noinst_HEADERS =      \
   gnc-event-p.h      \
   md5.h              \
   qofclass-p.h       \
   qofla-dir.h  \
   qofmath128.h  \
   qofquery-p.h       \
   qofquery-deserial.h     \
   qofquery-serialize.h    \
   qofbook-p.h        \
   qofobject-p.h      \
   qofquerycore-p.h   \
   qofsession-p.h

AM_CFLAGS = -g        \
   @CFLAGS@           \
	${LIBXML2_CFLAGS}  \
  ${LIBGDA_CFLAGS} \
  ${GLIB_CFLAGS} \
  -Werror \
  -Wmissing-prototypes \
  -Wmissing-declarations

if USE_LIBGDA
INCLUDES = \
  -I$(includedir) \
  -DPREFIX=\""$(prefix)"\"  \
  -DDATADIR=\""$(datadir)"\"
else
INCLUDES =                     \
   -I${top_srcdir}/lib/libsql             \
   -I$(includedir)             \
   -DPREFIX=\""$(prefix)"\"    \
   -DDATADIR=\""$(datadir)"\" 
endif

LDADD = \
  ${LIBGDA_LIBS} \
  ${GMODULE_LIBS} \
   ${GLIB_LIBS}

QOFLIBdir = $(libdir)

EXTRA_DIST = \
  qofla-dir.h.in \
  qofmath128.c

CONFIG_CLEAN_FILES = qofla-dir.h

qofla-dir.h: qofla-dir.h.in
	rm -f $@.tmp
	sed < $< > $@.tmp \
                -e 's:@-libdir-@:${QOFLIBdir}:g'
	mv $@.tmp $@

BUILT_SOURCES = qofla-dir.h
