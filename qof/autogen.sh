#!/bin/sh -x
set -e

echo autoreconf requires autopoint
autoreconf -ifs

echo "Now you can run ./configure --enable-error-on-warning --enable-compile-warnings"
