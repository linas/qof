
pkglib_LTLIBRARIES = libqof.la

libqof_la_SOURCES = 		\
	gncObject.c		\
	gnc-book.c		\
	gnc-date.c		\
	gnc-engine-util.c		\
	gnc-numeric.c		\
	gnc-event.c		\
	gnc-session.c		\
	gnc-trace.c		\
	guid.c			\
	kvp_frame.c     \
	md5.c           \
	Backend.c       \
	GNCId.c         \
	QueryCore.c     \
	QueryNew.c      \
	QueryObject.c 

noinst_HEADERS = 	\
	gncObject.h      \
	gncObjectP.h     \
	gnc-book.h       \
	gnc-book-p.h     \
	gnc-date.h       \
	gnc-engine-util.h		\
	gnc-numeric.h		\
	gnc-event.h		\
	gnc-event-p.h    \
	gnc-session.h		\
	gnc-session-p.h		\
	gnc-trace.h		\
	guid.h			\
	kvp_frame.h		\
	md5.h          \
	Backend.h        \
	BackendP.h       \
	GNCId.h          \
	GNCIdP.h         \
	QueryCore.h      \
	QueryCoreP.h     \
	QueryNew.h       \
	QueryNewP.h      \
	QueryObject.h    \
	QueryObjectP.h
	

CFLAGS = -g \
	@CFLAGS@ \
	@GLIB_CFLAGS@

INCLUDES = 			\
	-I$(includedir)		\
	-DPREFIX=\""$(prefix)"\" \
	-DDATADIR=\""$(datadir)"\" 
						 


