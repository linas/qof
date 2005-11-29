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
	if test ! -d ../website; then mkdir ../website; fi
	if test -h ../website/doxy; then rm ../website/doxy; fi	
	ln -s ../doc/html/ ../website/doxy


