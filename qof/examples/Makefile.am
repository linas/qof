
query_example_SOURCES = \
	query-example.c      \
	my-object.c 

sql_example_SOURCES = \
	sql-example.c      \
	my-object.c 

noinst_PROGRAMS = \
	query-example  \
	sql-example

EXTRA_DIST =                   \
    .cvsignore


AM_CFLAGS = \
  -I${top_srcdir}/src \
  ${GLIB_CFLAGS}


LDADD = \
  ${top_builddir}/src/libqof.la \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${GLIB_LIBS} \
  -lltdl -lm

