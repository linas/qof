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
	rm ../website/doxy
	ln -s ../doc/html/ ../website/doxy
