SUBDIRS = html

EXTRA_DIST = \
  README \
  doxygen_main_page.c \
  doxygen.cfg.in \
  backend-api.txt \
  backend-errors.txt \
  gnc-numeric-example.txt \
  qofdatelocale.txt \
  map-plan.txt

docdir = ${datadir}/doc

all: doc 

# 
# The doxy symlink is provided for CVS users and the SF site.
# Ignore the symlink when packaging website HTML alongside
# the Doxygen output.
#
doc: doxygen.cfg Makefile
	${RM} html/*.html
	${DOXYGEN} doxygen.cfg
	if test ! -d ../website; then mkdir ../website; fi
	if test -h ../website/doxy; then rm ../website/doxy; fi	
	ln -s ../doc/html/ ../website/doxy

clean-local:
	${RM} doxygen.log

qofdocdir = ${docdir}/qof/html/doxy

qofdoc_DATA = \
	html/*

install-data-hook:
	if test -f ${DESTDIR}${qofdocdir}/Makefile; then \
		rm ${DESTDIR}${qofdocdir}/Makefile; fi
	if test -f ${DESTDIR}${qofdocdir}/Makefile.am; then \
		rm ${DESTDIR}${qofdocdir}/Makefile.am; fi
	if test -f ${DESTDIR}${qofdocdir}/Makefile.in; then \
		rm ${DESTDIR}${qofdocdir}/Makefile.in; fi

qofdoc: doc
