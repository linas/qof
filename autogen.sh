#!/bin/sh
set -e

if [ ! -x /usr/bin/autopoint ]; then
	echo autoreconf requires autopoint because QOF uses gettext
	exit 1
fi
autoreconf -ifs
echo "Now you can run ./configure --enable-error-on-warning --enable-compile-warnings"

