# This is free software, licensed under the GNU Public License V2.
# See the file COPYING for details.

dnl # ******************************
dnl # Initialise
dnl # ******************************

AC_INIT(qof, 0.8.0, http://lists.sourceforge.net/lists/listinfo/qof-devel)

AC_PREREQ(2.53)
AC_GNU_SOURCE

LIBQOF_LIBRARY_VERSION=2:0:0
LIBQOFSQL_LIBRARY_VERSION=1:1:0

AC_SUBST(LIBQOF_LIBRARY_VERSION)
AC_SUBST(LIBQOFSQL_LIBRARY_VERSION)
SONAME=`echo $LIBQOF_LIBRARY_VERSION | cut -d: -f1`
AC_SUBST(SONAME)

AC_PREFIX([/usr])
VERSION=AC_PACKAGE_VERSION
AC_SUBST(VERSION)
PACKAGE=AC_PACKAGE_NAME
AC_SUBST(PACKAGE)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_LIBTOOL
AM_CONFIG_HEADER(config.h)

AC_ISC_POSIX
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_STDC_HEADERS
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_FUNC_MKTIME
AC_FUNC_STRTOD

dnl # *****************************************
dnl # pkg-config check time
dnl # *****************************************

AM_PATH_GLIB_2_0("2.6.0", , ,gobject)

AC_PATH_PROG(PKG_CONFIG,pkg-config)
if test "x$PKG_CONFIG" != x; then
	GLIB_LIBS=`$PKG_CONFIG --libs glib-2.0`
	GLIB_CFLAGS=`$PKG_CONFIG --cflags glib-2.0`
	GOBJECT_LIBS=`$PKG_CONFIG --libs gobject-2.0`
	GMODULE_LIBS=`$PKG_CONFIG --libs gmodule-2.0`
fi
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GOBJECT_LIBS)
AC_SUBST(GMODULE_LIBS)

AC_MSG_CHECKING(for GLIB - version >= 2.9.0)
if $PKG_CONFIG 'glib-2.0 >= 2.9.0'
then
	AC_MSG_RESULT(yes)
	AC_DEFINE(HAVE_GLIB29,1,[System has glib 2.9.0 or better])
	HAVE_GLIB29=yes
else
	AC_MSG_RESULT(no)
fi

dnl # ********************************
dnl # Symbol versioning support
dnl # ********************************

dnl # use symbol versioning on Linux, to avoid problems when
dnl # mixing-and-matching multiple lib versions; may be useful on other
dnl # platforms that support symbol versioning (BSDs?)

AC_MSG_CHECKING(whether to implement symbol versioning)
case $host_os in
  *gnu*)
	LINKER_SCRIPT="-Wl,--version-script=\$(srcdir)/libqof.ver"
	AC_MSG_RESULT(yes)
	;;
  *)
	AC_MSG_RESULT(no)
	;;
esac
AC_SUBST(LINKER_SCRIPT)

dnl # ********************************
dnl # Auto-disable deprecated QOF
dnl # ********************************

# nothing to do, yet.

AC_MSG_CHECKING(for darwin)
case $host_os in
	rhapsody* | darwin1*)
		AC_MSG_RESULT([yes, but too old])
		AC_MSG_ERROR([This platform is too old, please update to the latest darwin])
		;;
	darwin*)
		AC_MSG_RESULT([yes, patching libtool to always build dylibs])
		mv libtool libtool.old
		sed -e 's/^deplibs_check_method.*/deplibs_check_method=pass_all/g' \
			-e 's|^library_names_spec.*|library_names_spec="\\$libname\\$release\\$versuffix.dylib \\$libname\\$release\\${major}.dylib \\$libname.dylib"|g' \
			-e 's|^soname_spec.*|soname_spec="\\$libname\\$release\\$major.dylib"|g' \
			< libtool.old > libtool
		rm libtool.old
		AC_DEFINE(DARWIN,,Found Mac OSX Darwin)
		DARWIN="dylib"
		AC_SUBST(DARWIN)
		;;
	*)
		AC_MSG_RESULT(no)
		;;
esac

dnl # **************************************************************
dnl # Checks for typedefs, structures, and compiler characteristics.
dnl # **************************************************************

AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T

dnl # *****************************
dnl # Checks for library functions.
dnl # *****************************
AC_FUNC_STRFTIME
AC_CHECK_HEADERS(sys/times.h time.h langinfo.h wchar.h )
AC_CHECK_FUNCS(getcwd gettimeofday getline getwd stpcpy strdup strtoul \
	strcasestr gmtime_r mblen tzname tzset)
AC_CHECK_MEMBERS([struct stat.st_rdev])

dnl # *******************************
dnl # Internationalization
dnl # *******************************

GETTEXT_PACKAGE=qof
AC_SUBST(GETTEXT_PACKAGE)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"], [gettext domain])

dnl # ********************************
dnl # Auto-disable deprecated glib
dnl # ********************************

AC_ARG_ENABLE(deprecated-glib,
  [  --disable-deprecated-glib
                          Do not use deprecated glib functions],
  [case "${enableval}" in
        no) AC_DEFINE(G_DISABLE_DEPRECATED,1,
          [Do not use deprecated glib functions]) ;;
        yes) ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-deprecated-glib) ;;
        esac])

dnl # **************************************************************
dnl # Embedded systems only use the QofSQLite backend.
dnl # libxml2 and optional sqlite for host systems.
dnl # libxml2 v2.5.10 is required for QSF (backend/file)
dnl # libgdasql is omitted to prevent a need for libgda2-3 itself.
dnl # **************************************************************

AC_ARG_ENABLE(embedded,
	[  --enable-embedded       Enable the SQLite backend for embedded systems
                          and DISABLE libgdasql and the QSF XML backend. (no)],
	[case "${enableval}" in
		no) embedded="no" ;;
		yes) embedded="yes" ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-embedded) ;;
		esac])

if test "$embedded" = "yes"; then
	PKG_CHECK_MODULES(SQLITE, sqlite >= 2.8.0)
	AC_SUBST(SQLITE_LIBS)
	AC_SUBST(SQLITE_CFLAGS)
	backend="sqlite"
else
	LIBXML2_REQUIRED=2.5.10
	PKG_CHECK_MODULES(LIBXML2, libxml-2.0 >= $LIBXML2_REQUIRED)
	LIBXML_VERSION=`xml2-config --version`
	AC_SUBST(LIBXML2_CFLAGS)
	AC_SUBST(LIBXML2_LIBS)
	backend="libxml2 == $LIBXML_VERSION"
fi

AM_CONDITIONAL(EMBEDDED, [test "$embedded" = "yes"])

sqlite="yes"
AC_ARG_ENABLE(sqlite,
  [  --disable-sqlite        Disable the SQLite backend module.],
  [case "${enableval}" in
  		no) sqlite="no" ;;
		yes) sqlite="yes" ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --disable-sqlite) ;;
		esac])

if test "$embedded" != "yes"; then
	if test "$sqlite" = "yes"; then
		SQLITE_REQUIRED=2.8.0
		PKG_CHECK_MODULES(SQLITE, sqlite >= $SQLITE_REQUIRED)
		AC_SUBST(SQLITE_LIBS)
		AC_SUBST(SQLITE_CFLAGS)
		backend2="sqlite"
	fi
fi

AM_CONDITIONAL(USE_SQLITE, [test "$sqlite" = "yes"])

dnl # **************************************************************
dnl # GNOME Data Access library for GNOME2 libgda
dnl # Enables use of gdasql within libqof1 *and* the gda backend.
dnl # **************************************************************

dnl # Not ready yet.
usegdabackend="no"
backend3="Disabled (default)"
if test "$embedded" != "yes"; then
	usegdasql="yes"
else
	usegdasql="no"
	usegdabackend="no"
fi
AC_ARG_ENABLE(gdasql,
  [  --disable-gdasql        Disable the use of libgdasql.],
  [case "${enableval}" in
		no) usegdasql="no" ;;
		yes) usegdasql="yes" ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --disable-gda) ;;
		esac])

if test "$usegdasql" = "yes"; then
	AC_MSG_CHECKING(for SQL parser support)
	if test "$embedded" != "yes"; then
		GDA_REQUIRED=3.0.1
		LIBGDA_CFLAGS=`$PKG_CONFIG --cflags "libgda-3.0 >= $GDA_REQUIRED"`
		LIBGDA_LIBS="-lgda-3.0"
		# LIBGDA should only be used for the GDA backend
		AC_SUBST(LIBGDA_CFLAGS)
		AC_SUBST(LIBGDA_LIBS)
	fi
fi
if test "x$LIBGDA_CFLAGS" = x; then
	AC_MSG_RESULT([GDA not found (or disabled); will build using internal SQL library.])
	SQL_PKG_LIB="-lqofsql"
else
	AC_ARG_ENABLE(gdabackend,
	  [  --enable-gdabackend     Enable the use of the gda backend (off).],
	  [case "${enableval}" in
			no) usegdabackend="no" ;;
			yes) usegdabackend="yes" ;;
			*) AC_MSG_ERROR(bad value ${enableval} for --enable-gdabackend) ;;
			esac])
	GDA_VERSION=`$PKG_CONFIG --modversion libgda-3.0`
	AC_SUBST(GDA_VERSION)
	AC_MSG_RESULT([yes, using GDA: ${GDA_VERSION}])
	save_LIBS="$LIBS"
	AC_CHECK_LIB(gda-3.0, gda_client_open_connection, ,
		[AC_MSG_ERROR([libgda3-3 found but missing gda_client_open_connection])])
	AC_MSG_CHECKING(for libgdasql-3.0 support)
	AC_CHECK_LIB(gdasql-3.0, sql_parse, , [AC_MSG_ERROR([libgda3-3 found but missing libgdasql-3.0])])
	AC_DEFINE(HAVE_GDA,,[We have found GDA])
	GDA_PKG_LIB="-lgdasql-3.0"
	LIBS="$save_LIBS"
	HAVE_GDA="use_gda"
	if test "$usegdabackend" = "yes"; then
		backend3="Enabled"
	fi
fi

AM_CONDITIONAL(USE_GDABACKEND, [ test x"$usegdabackend" != x"no"])

AM_CONDITIONAL(USE_LIBGDA, [ test x"$HAVE_GDA" != x])
AM_CONDITIONAL(USE_LIBGDA, [ test "$HAVE_GDA" = "use_gda"])
AC_SUBST(USE_LIBGDA)
AC_SUBST(GDA_PKG_LIB)
AC_SUBST(SQL_PKG_LIB)

dnl # **************************************************************
dnl # DWI is now unsupported.
dnl # The DWI code is still packaged for now
dnl # but will be removed in libqof2.
dnl # **************************************************************

dnl # =========================
dnl # Perl - no longer needed
dnl # =========================

# check for doxygen, mostly stolen from http://log4cpp.sourceforge.net/
# ------------------------------------------------------------------
AC_DEFUN([BB_ENABLE_DOXYGEN],
[
AC_ARG_ENABLE(doxygen, [  --enable-doxygen        enable documentation generation with doxygen (auto)])
AC_ARG_ENABLE(dot, [  --enable-dot            use 'dot' to generate graphs in doxygen (auto)])
AC_ARG_ENABLE(html-docs, [  --enable-html-docs      enable HTML generation with doxygen (yes)], [], [ enable_html_docs=yes])
AC_ARG_ENABLE(latex-docs, [  --enable-latex-docs     enable LaTeX documentation generation with doxygen (no)], [], [ enable_latex_docs=no])
if test "$embedded" = "yes"; then
	enable_doxygen=no
	enable_doc=no
fi
if test "x$enable_doxygen" = xno; then
        enable_doc=no
else
        AC_PATH_PROG(DOXYGEN, doxygen, , $PATH)
        if test x$DOXYGEN = x; then
                if test "x$enable_doxygen" = xyes; then
                        AC_MSG_ERROR([could not find doxygen])
                fi
                enable_doc=no
        else
                enable_doc=yes
                AC_PATH_PROG(DOT, dot, , $PATH)
        fi
fi
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)

if test x$DOT = x; then
        if test "x$enable_dot" = xyes; then
                AC_MSG_ERROR([could not find dot])
        fi
        enable_dot=no
else
        enable_dot=yes
fi
AM_CONDITIONAL(ENABLE_DOXYGEN, test "$enable_doc" = "yes")
AC_SUBST(enable_dot)
AC_SUBST(enable_html_docs)
AC_SUBST(enable_latex_docs)
])

# check for doxygen
BB_ENABLE_DOXYGEN
# ----------------------------------------------------------------

dnl # *************************************
dnl # Extra warnings
dnl # *************************************
AC_MSG_CHECKING(what extra warning flags to pass to the C compiler)
if test ${GCC}x = yesx; then
  warnFLAGS=
  CFLAGS="${CFLAGS} -g2 -Wall"
  AC_ARG_ENABLE(error-on-warning,
        [  --disable-error-on-warning
                          disable treating compile warnings as errors],
        [case "${enableval}" in
        yes) warnFLAGS="${warnFLAGS} -Werror" ;;
        no)  ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-error-on-warning) ;;
        esac],
        [  warnFLAGS="${warnFLAGS} -Werror" ])
  GCC_VERSION=`${CC} -dumpversion`
  if test `echo ${GCC_VERSION} | cut -d. -f1` -ge 3; then
     # This is gcc >= 3.x.x
     if test `echo ${GCC_VERSION} | cut -d. -f2` -ge 4; then
        # This is gcc >= 3.4.x
        warnFLAGS="${warnFLAGS} -Wdeclaration-after-statement"
     else if test `echo ${GCC_VERSION} | cut -d. -f1` -ge 4; then
        # This is gcc == 4.x.x
        warnFLAGS="${warnFLAGS} -Wdeclaration-after-statement -Wno-pointer-sign"
        fi
     fi
  fi
dnl  CFLAGS="${CFLAGS} ${warnFLAGS}"
else
  warnFLAGS=
fi
AC_MSG_RESULT($warnFLAGS)
AC_SUBST(warnFLAGS)

set_compile_warnings=no

AC_ARG_ENABLE(compile-warnings,
              [  --enable-compile-warnings
                          Enable verbose compiler warnings],
              set_compile_warnings=yes)

if test "$GCC" = "yes" -a "$set_compile_warnings" != "no"; then
        warnFLAGS="$warnFLAGS -Wcast-align -Wsign-compare \
 -Wmissing-prototypes -Wmissing-declarations -Werror -Wmissing-include-dirs \
 -Wstrict-prototypes -Winline -Wnested-externs -Wpointer-arith \
 -Wmissing-field-initializers"
# LDFLAGS="$LDFLAGS -Wl,-z,defs"
fi

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

DATADIR=`eval echo $datadir`
QSF_SCHEMA_DIR=`eval echo $DATADIR`"/xml/qof/qsf"
AC_SUBST(QSF_SCHEMA_DIR)
BACKEND_SCHEMA_DIR=`eval echo $DATADIR`"/xml/qof/"
AC_SUBST(BACKEND_SCHEMA_DIR)

QOF_CFLAGS="-I$includedir/qof"
QOF_LIBS="-lqof"
QOF_LIBDEPS="$LDFLAGS"
QOF_VERSION="$VERSION"

QSF_LIB_DIR=`eval echo $libdir | sed "s%^NONE%$prefix%"`
QSF_LIB_DIR=`eval echo $QSF_LIB_DIR/qof/ | sed "s%^NONE%$prefix%"`
AC_SUBST(QSF_LIB_DIR)
AC_SUBST(QOF_LIBS)

dnl # ******************************
dnl # Check if this was CVS source
dnl # ******************************
if test -f "${srcdir}/README.cvs" ; then
	AC_DEFINE([TEST_DEBUG],1,
		[Run test code in debug mode])
	QOF_FROM_CVS=yes
fi

AC_OUTPUT([ po/Makefile.in
Makefile
backend/Makefile
backend/file/Makefile
backend/gda/Makefile
backend/file/qsf-dir.h
backend/sqlite/Makefile
doc/Makefile
doc/html/Makefile
doc/doxygen.cfg
lib/Makefile
lib/libsql/Makefile
qof/Makefile
qof/qofla-dir.h
qof/test/Makefile
qof.pc
qof.symbols
])

dnl # <= Configuration summary =>

echo ""
echo "qof configuration summary:"
echo "==========================="
echo ""

dnl # <= CFLAGS and LDFLAGS =>
echo "CFLAGS      :   $warnFLAGS"
echo "LDFLAGS     :   $LDFLAGS"
echo "QOF_CFLAGS  :   "`eval echo $QOF_CFLAGS`
echo "QOF_LIBS    :   $QOF_LIBS"
echo "DWI_LIBS    :   $DWI_LIBS"
echo "1st backend :   $backend"
echo "2nd backend :   $backend2"
echo "GDA backend :   $backend3"
echo "libgda      :   $GDA_VERSION"
echo "libgdasql   :   $GDA_PKG_LIB"
echo "libqofsql   :   $SQL_PKG_LIB"
echo "prefix      :   $prefix"
echo "data dir    :   "`eval echo $DATADIR`
echo "XML dir     :   "`eval echo $QSF_SCHEMA_DIR`
echo "include dir :   "`eval echo $includedir`
echo "lib dir     :   "`eval echo $QSF_LIB_DIR`
echo ""
echo ""
dnl # <= End of configuration summary =>

if [[ x"$QOF_FROM_CVS" = xyes ]]; then
  echo " .----- NOTICE ------------------------------------------------."
  echo " |         You are using QOF from CVS source.                  |"
  echo " |                                                             |"
  echo " | This is likely to be unstable, or contain some incomplete   |"
  echo " | features, or just plain not work at all. Use it at your own |"
  echo " | risk. Please help me to fix any bugs you find, by reporting |"
  echo " | them back to me via the QOF-devel mailing list.             |"
  echo " | http://lists.sourceforge.net/lists/listinfo/qof-devel       |"
  echo " \`-------------------------------------------------------------'"
  echo ""
fi
