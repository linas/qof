#!/bin/sh -x

# Exit this script if any command fails with non-zero exit status.
set -e

# First cache the command names in variables. If you want to
# override the names, simply set the variables before calling this
# script.

: ${GETTEXTIZE=gettextize}
: ${LIBTOOLIZE=libtoolize}
: ${ACLOCAL=aclocal}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${AUTOCONF=autoconf}

if [ ! -e ABOUT-NLS ]; then
	rm intltool*
	${GETTEXTIZE} -f
	rm po/Makevars.template
fi
${LIBTOOLIZE} --force --automake
${ACLOCAL} ${ACLOCAL_FLAGS}
${AUTOHEADER}
${AUTOMAKE} --add-missing
${AUTOCONF}

echo "Now you can run ./configure --enable-error-on-warning --enable-compile-warnings"

