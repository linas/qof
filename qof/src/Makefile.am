
pkglib_LTLIBRARIES = libqof.la

libqof_la_SOURCES = 		\
	gnc-date.c		\
	gnc-engine-util.c		\
	gnc-numeric.c		\
	gnc-event.c		\
	gnc-trace.c		\
	guid.c			\
	kvp_frame.c     \
	md5.c           \
	qofbackend.c    \
	qofid.c         \
	qofquery.c      \
	qofqueryobject.c \
	qofbook.c        \
	qofobject.c      \
	qofquerycore.c   \
	qofsession.c

qofincludedir = ${INCLUDE_DIR}
qofinclude_HEADERS = \
	gnc-date.h       \
	gnc-engine-util.h	\
	gnc-numeric.h		\
	gnc-event.h		  \
	gnc-trace.h		  \
	guid.h			  \
	kvp_frame.h		  \
	md5.h            \
	qofbackend.h    \
	qofid.h         \
	qofquery.h      \
	qofqueryobject.h \
	qofbook.h        \
	qofobject.h      \
	qofquerycore.h   \
	qofsession.h


noinst_HEADERS = 	\
	gnc-event-p.h    \
	qofbackend-p.h    \
	qofid-p.h         \
	qofquery-p.h      \
	qofqueryobject-p.h \
	qofbook-p.h        \
	qofobject-p.h      \
	qofquerycore-p.h   \
	qofsession-p.h

	
EXTRA_DIST =                   \
   .cvsignore


CFLAGS = -g \
	@CFLAGS@ \
	@GLIB_CFLAGS@

INCLUDES = 			\
	-I$(includedir)		\
	-DPREFIX=\""$(prefix)"\" \
	-DDATADIR=\""$(datadir)"\" 
						 


