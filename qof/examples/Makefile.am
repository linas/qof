
query_example_SOURCES = \
	my-object.c          \
	query-example.c

noinst_PROGRAMS = \
	query-example

EXTRA_DIST =                   \
    .cvsignore


AM_CFLAGS = \
  -I${top_srcdir}/src \
  ${GLIB_CFLAGS}


LDADD = \
  ${top_builddir}/src/libqof.la \
  ${GLIB_LIBS} \
  -lltdl -lm

