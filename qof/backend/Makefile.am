#
# FILE:
# Makefile.am
#
# FUNCTION:
# QOF backend builder
#
# HISTORY:
# Created by Linas Vepstas <linas@linas.org> July 2004
# Conditional DWI build: Neil Williams <linux@codehelp.co.uk>
# QSF added: Neil Williams <linux@codehelp.co.uk> Dec 2004
# QSQLite added: Neil Williams <linux@codehelp.co.uk> Aug 06

if BUILD_DWI
DWIBE=dwi
else
DWIBE=
endif

if EMBEDDED
SQLITE=sqlite
else
SQLITE=file
endif

SUBDIRS = $(DWIBE) $(SQLITE)

DIST_SUBDIRS =  dwi file sqlite
 

