#
# Makefile.am for libqof
#
if USE_LIBGDA
SUBDIRS = backend qof doc examples
else
SUBDIRS = lib backend qof doc examples
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof-1.pc

EXTRA_DIST =                   \
	AUTHORS                     \
	COPYING                     \
	ChangeLog                   \
	INSTALL                     \
	NEWS                        \
	README                      \
	TODO                        \
	autogen.sh                  \
	intltool-extract.in         \
	intltool-merge.in           \
	intltool-update.in          \
	qof-1.pc.in

DISTCLEANFILES = \
	intltool-update \
	intltool-merge    \
	intltool-extract
