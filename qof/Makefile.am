SUBDIRS = lib backend qof . po doc

DIST_SUBDIRS = lib backend qof . po doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof-1.pc

noinst_DATA = make-potfiles

make-potfiles: make-potfiles.in Makefile
	${RM} $@.tmp
	sed < $< > $@.tmp \
      -e 's:@-PERL-@:${PERL}:g'
	chmod +x $@.tmp
	mv $@.tmp $@

POTFILES: make-potfiles Makefile 
	${RM} ./po/POTFILES.in
	./make-potfiles > ./po/POTFILES.in

BUILT_SOURCES=POTFILES make-potfiles

pot: Makefile
	${MAKE} make-potfiles
	${MAKE} POTFILES
	${MAKE} -C po $(PACKAGE).pot

all-local: pot

EXTRA_DIST = config.rpath m4/ChangeLog  \
  HACKING \
  make-potfiles.in \
  po/POTFILES.in \
  po/POTFILES.skip \
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
  make-potfiles \
  po/POTFILES.in

ACLOCAL_AMFLAGS = -I m4
