SUBDIRS = lib backend qof . po doc

DIST_SUBDIRS = lib backend qof . po doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof-1.pc

pot: Makefile
	${MAKE} -C po $(PACKAGE).pot

all-local: pot

EXTRA_DIST = config.rpath m4/ChangeLog  \
  HACKING \
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

ACLOCAL_AMFLAGS = -I m4
