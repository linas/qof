if ENABLE_DOXYGEN
install-data-local: doc
else
install-data-local:
endif

SUBDIRS = html

EXTRA_DIST = \
  README \
  doxygen_main_page.c \
  doxygen.cfg.in \
  backend-api.txt \
  backend-errors.txt \
  qofnumeric-example.txt \
  qofdatelocale.txt \
  map-plan.txt

# The doxy symlink is provided for CVS users and the SF site.
# Ignore the symlink when packaging website HTML alongside
# the Doxygen output.

if ENABLE_DOXYGEN
doc: doxygen.cfg Makefile
	${RM} html/*.html
	${DOXYGEN} doxygen.cfg
	if test ! -d ../website; then mkdir ../website; fi
	if test -h ../website/doxy; then rm ../website/doxy; fi	
	ln -s ../doc/html/ ../website/doxy

qofdocdir = ${datadir}/doc/qof/html/doxy
qofdoc_DATA = html/*
qofdoc: doc
install-data-hook:
	if test -f ${DESTDIR}${qofdocdir}/Makefile; then \
		rm ${DESTDIR}${qofdocdir}/Makefile; fi
	if test -f ${DESTDIR}${qofdocdir}/Makefile.am; then \
		rm ${DESTDIR}${qofdocdir}/Makefile.am; fi
	if test -f ${DESTDIR}${qofdocdir}/Makefile.in; then \
		rm ${DESTDIR}${qofdocdir}/Makefile.in; fi
else
doc:
qofdocdir = .
qofdoc_DATA =
qofdoc:
install-data-hook:
endif

clean-local:
	${RM} doxygen.log
	if test -h ../website/doxy; then rm ../website/doxy; fi

