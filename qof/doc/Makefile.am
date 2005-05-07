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
	echo "doc: " && pwd
	rm -rf html
	$(DOXYGEN) doxygen.cfg

#	$(MAKE) -C latex/ 
#	mv latex/refman.pdf ./refman.pdf

#dist-hook:

#distclean-local:

#maintainer-clean-local:

#install-data-hook:

#uninstall-hook:

## We borrow guile's convention and use @-...-@ as the substitution
## brackets here, instead of the usual @...@.  This prevents autoconf
## from substituting the values directly into the left-hand sides of
## the sed substitutions.
doxygen.cfg: doxygen.cfg.in Makefile
	rm -f $@.tmp
	sed < $< > $@.tmp \
            -e 's:@-VERSION-@:${QOF_VERSION}:g'
	mv $@.tmp $@
