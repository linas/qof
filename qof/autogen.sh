#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0` 
test -z "$srcdir" && srcdir=.

PKG_NAME="QOF"

(test -f $srcdir/configure.in \
  && test -f $srcdir/ChangeLog \
  && test -d $srcdir/src) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level qof directory"
    exit 1
}

# uhh, we should get rid of this, since this package shouldn't need gnome
which gnome-autogen.sh || {
    echo "You need to install gnome-common"
    exit 1
}
USE_GNOME2_MACROS=1 . gnome-autogen.sh
