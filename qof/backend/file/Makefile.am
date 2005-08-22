SUBDIRS = .

lib_LTLIBRARIES=libqof-backend-qsf.la

AM_CFLAGS = \
	-I.. -I../.. \
	-I${top_srcdir}/qof \
	${LIBXML2_CFLAGS} \
	${GLIB_CFLAGS}

libqof_backend_qsf_la_SOURCES = \
	qsf-backend.c \
	qsf-xml-map.c \
	qsf-xml.c

libqof_backend_qsf_la_LDFLAGS = -version-info $(LIBQOF_BACKEND_QSF_LIBRARY_VERSION)
	
libqof_backend_qsf_la_LIBADD = \
	${LIBXML2_LIBS} \
	${GLIB_LIBS}

qofincludedir = ${pkgincludedir}

qofinclude_HEADERS = \
	qof-backend-qsf.h

qsfschemadir = $(QSF_SCHEMA_DIR)
qsfschema_DATA = \
        qsf-object.xsd.xml \
        qsf-map.xsd.xml \
  pilot-qsf-GnuCashInvoice.xml \
  qsf-backend-v0.1.xml

EXTRA_DIST = \
  $(qsfschema_DATA) \
  qsf-dir.h.in    \
  backend-config.pl \
  qsf-xml.h

CONFIG_CLEAN_FILES = \
  qsf-dir.h \
  qsf-backend-v0.1.xml

qsf-dir.h: qsf-dir.h.in
	rm -f $@.tmp
	sed < $< > $@.tmp \
		-e 's:@-QSF_SCHEMA_DIR-@:${QSF_SCHEMA_DIR}:g'
	mv $@.tmp $@

qsf-backend-v0.1.xml: backend-config.pl
	perl backend-config.pl > qsf-backend-v0.1.xml

BUILT_SOURCES = \
  qsf-dir.h \
  qsf-backend-v0.1.xml

