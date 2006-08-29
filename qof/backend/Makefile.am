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

# Always build QSQLite, only build XML if *not* embedded.

if BUILD_DWI
DWIBE=dwi
else
DWIBE=
endif

if EMBEDDED
XML=
else
XML=file
endif

SUBDIRS = $(DWIBE) $(XML) sqlite

DIST_SUBDIRS =  dwi file sqlite
 

