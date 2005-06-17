
gobj_example_SOURCES = \
	gobj-example.c 

noinst_PROGRAMS =     \
	gobj-example 

EXTRA_DIST =


AM_CFLAGS =           \
	-I${top_srcdir}/qof \
  ${GTK_CFLAGS}       \
  ${GLIB_CFLAGS}

LDADD =                                   \
  ${top_builddir}/qof/libqof.la           \
  ${top_builddir}/lib/libsql/libqofsql.la \
  ${GTK_LIBS}                             \
  ${LIBXML2_LIBS}                         \
  ${GLIB_LIBS}                            \
  -lltdl -lm

