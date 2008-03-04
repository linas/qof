#!/bin/sh -x
set -e

autoreconf -ifs

echo "Now you can run ./configure --enable-error-on-warning --enable-compile-warnings"
