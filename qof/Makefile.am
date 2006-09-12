SUBDIRS = lib backend qof . po doc

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

EXTRA_DIST = \
  HACKING \
  make-potfiles.in \
  po/POTFILES.in \
  po/POTFILES.skip \
  intltool-extract.in \
  intltool-merge.in \
  intltool-update.in \
  qof-1.pc.in \
  website/gobj-example.c.html \
  website/my-obj-example.c.html \
  website/sflogosm.png \
  website/sflogolg.png \
  website/hacking.html \
  website/qof.css \
  website/why-qof.html \
  website/favicon.png \
  website/index.html \
  website/qof-logo.jpg

DISTCLEANFILES = \
  intltool-update \
  intltool-merge    \
  intltool-extract \
  make-potfiles \
  po/POTFILES.in \
  po/.intltool-merge-cache
