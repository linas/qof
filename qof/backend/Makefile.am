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
# Added -Werror Neil Williams <linux@codehelp.co.uk> August 05
#

if BUILD_DWI
DWIBE=dwi
else
DWIBE=
endif

SUBDIRS = $(DWIBE) file

DIST_SUBDIRS =  dwi file
 
CFLAGS += -Werror

backendschemadir = $(BACKEND_SCHEMA_DIR)
backendschema_DATA = backend-schema.xsd.xml

EXTRA_DIST = \
 $(backendschema_DATA)

