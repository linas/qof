#
# Makefile.am for the libqof sources
#


# pkglib_LTLIBRARIES = libqof.la

lib_LTLIBRARIES = libqof.la

libqof_la_LDFLAGS= -version-info $(LIBQOF_LIBRARY_VERSION) -release $(VERSION)


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
   qofquery-deserial.c     \
   qofquery-serialize.c    \
   qofsession.c      \
   qofsql.c

# qofconfigdir = ${pkgincludedir}
# qofconfig_DATA = config.h

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
   qofquery-deserial.h     \
   qofquery-serialize.h    \
   qofsession.h      \
   qofsql.h


noinst_HEADERS =      \
   gnc-event-p.h      \
   kvp-util-p.h       \
   md5.h              \
   qofbackend-p.h     \
   qofclass-p.h       \
   qofquery-p.h       \
   qofbook-p.h        \
   qofobject-p.h      \
   qofquerycore-p.h   \
   qofsession-p.h

   
EXTRA_DIST =          \
   .cvsignore


AM_CFLAGS = -g        \
   @CFLAGS@           \
	${LIBXML2_CFLAGS}  \
   ${GLIB_CFLAGS}

INCLUDES =                     \
   -I../lib                    \
   -I$(includedir)             \
   -DPREFIX=\""$(prefix)"\"    \
   -DDATADIR=\""$(datadir)"\" 
                   

