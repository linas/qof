SUBDIRS = lib qof . backend po doc

DIST_SUBDIRS = lib qof . backend po doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qof.pc

all-local: Makefile
	ln -sf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)_$(VERSION).orig.tar.gz

clean-local:
	find . -name '*~'|xargs rm -f

EXTRA_DIST = config.rpath \
  po/POTFILES.skip \
  qof.pc.in \
  qofsql.symbols

ACLOCAL_AMFLAGS = -I m4
