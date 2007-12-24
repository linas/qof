SUBDIRS = lib qof . backend po doc

DIST_SUBDIRS = lib qof . backend po doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof-1.pc qof.pc

#pot: Makefile
#	${MAKE} -C po $(PACKAGE).pot

#all-local: pot

EXTRA_DIST = config.rpath m4/ChangeLog  \
  po/POTFILES.skip \
  qof-1.pc.in \
  qof.pc.in \
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
