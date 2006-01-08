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
	./make-potfiles > ./po/POTFILES.in

pot: Makefile
	rm -f po/$(PACKAGE).pot
	make -C po $(PACKAGE).pot

EXTRA_DIST =                   \
  HACKING \
  make-potfiles.in \
  po/POTFILES.in \
	intltool-extract.in         \
	intltool-merge.in           \
	intltool-update.in          \
	qof-1.pc.in

DISTCLEANFILES = \
	intltool-update \
	intltool-merge    \
  intltool-extract \
  make-potfiles \
  po/POTFILES.in \
  po/.intltool-merge-cache
