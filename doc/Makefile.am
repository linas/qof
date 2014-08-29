if ENABLE_DOXYGEN
install-data-local: doc
else
install-data-local:
endif

SUBDIRS = html

EXTRA_DIST = \
  README \
  qof.devhelp2 \
  doxygen_main_page.c \
  doxygen_to_devhelp.xsl \
  doxygen.cfg.in \
  backend-api.txt \
  backend-errors.txt \
  qofnumeric-example.txt \
  qofdatelocale.txt \
  map-plan.txt

if ENABLE_DOXYGEN
doc: doxygen.cfg Makefile qof.devhelp2 xml/index.xml

xml/index.xml:
	$(RM) html/*.html
	$(DOXYGEN) doxygen.cfg

qof.devhelp2: xml/index.xml
	xsltproc -o qof.devhelp2 $(top_srcdir)/doc/doxygen_to_devhelp.xsl xml/index.xml

qofdocdir = ${datadir}/doc/qof/html/
qofdoc_DATA = html/* \
  qof.devhelp2

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
	$(RM) doxygen.log
	$(RM) -r xml/
	$(RM) qof.devhelp2
