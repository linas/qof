#
# Makefile.am for libqof
#
if USE_LIBGDA
SUBDIRS = backend qof . po doc
else
SUBDIRS = lib backend qof . po doc
endif

DIST_SUBDIRS = lib backend qof . po doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof-1.pc

noinst_DATA = make-potfiles

make-potfiles: make-potfiles.in Makefile
	rm -f $@.tmp
	sed < $< > $@.tmp \
      -e 's:@-PERL-@:${PERL}:g'
	chmod +x $@.tmp
	mv $@.tmp $@

EXTRA_DIST =                   \
  make-potfiles.in \
	intltool-extract.in         \
	intltool-merge.in           \
	intltool-update.in          \
	qof-1.pc.in

DISTCLEANFILES = \
	intltool-update \
	intltool-merge    \
  intltool-extract \
  make-potfiles \
  po/.intltool-merge-cache
