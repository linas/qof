
# pkglib_LTLIBRARIES = libqof.la

lib_LTLIBRARIES = libqof.la

libqof_la_SOURCES =  \
   gnc-date.c        \
   gnc-engine-util.c \
   gnc-numeric.c     \
   gnc-event.c       \
   gnc-trace.c       \
   guid.c            \
   kvp_frame.c       \
   md5.c             \
   qofbackend.c      \
   qofclass.c        \
   qofid.c           \
   qofquery.c        \
   qofbook.c         \
   qofobject.c       \
   qofquerycore.c    \
   qofsession.c      \
   qofsql.c

qofincludedir = ${pkgincludedir}
qofinclude_HEADERS = \
   gnc-date.h        \
   gnc-engine-util.h \
   gnc-numeric.h     \
   gnc-event.h       \
   gnc-trace.h       \
   guid.h            \
   kvp_frame.h       \
   qof.h             \
   qofbackend.h      \
   qofclass.h        \
   qofid.h           \
   qofquery.h        \
   qofbook.h         \
   qofobject.h       \
   qofquerycore.h    \
   qofsession.h      \
   qofsql.h


noinst_HEADERS =      \
   gnc-event-p.h      \
   md5.h              \
   qofbackend-p.h     \
   qofclass-p.h       \
   qofid-p.h          \
   qofquery-p.h       \
   qofbook-p.h        \
   qofobject-p.h      \
   qofquerycore-p.h   \
   qofsession-p.h

   
EXTRA_DIST =          \
   .cvsignore


AM_CFLAGS = -g        \
   @CFLAGS@           \
   @GLIB_CFLAGS@

INCLUDES =                     \
   -I../lib                    \
   -I$(includedir)             \
   -DPREFIX=\""$(prefix)"\"    \
   -DDATADIR=\""$(datadir)"\" 
                   


