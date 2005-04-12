# Makefile for documentation

EXTRA_DIST = \
  README \
  doxygen_main_page.c \
  doxygen.cfg.in \
  backend-api.txt \
  backend-errors.txt 


docdir = ${datadir}

all:

doc: doxygen.cfg
	echo "doc: " && pwd && 	echo "distdir: " && echo $(distdir)
	rm -rf html refman.pdf
	$(DOXYGEN) doxygen.cfg
#	$(MAKE) -C latex/ 
#	mv latex/refman.pdf ./refman.pdf

dist-hook: doc
	echo "dist-hook: " && pwd
	cp -rp html ${distdir}

distclean-local:
	echo "distclean-local: " && pwd
	rm -f doxygen.cfg doxygen.log

maintainer-clean-local:
	echo "maintainer-clean-local: " && pwd
	rm -rf html latex
	rm -f refman.pdf

install-data-hook:
	$(mkinstalldirs) $(docdir)/html
	mkdir -p html  
	cp -rp html $(docdir)/html

uninstall-hook:
	rm -rf ${docdir}/html

## We borrow guile's convention and use @-...-@ as the substitution
## brackets here, instead of the usual @...@.  This prevents autoconf
## from substituting the values directly into the left-hand sides of
## the sed substitutions.
doxygen.cfg: doxygen.cfg.in Makefile
	rm -f $@.tmp
	sed < $< > $@.tmp \
            -e 's:@-VERSION-@:${VERSION}:g'
	mv $@.tmp $@

