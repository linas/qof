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

noinst_HEADERS = qsf-dir.h

libqof_backend_qsf_la_LIBADD = \
	${LIBXML2_LIBS} \
	${GLIB_LIBS}

qsfschemadir = $(QSF_SCHEMA_DIR)
qsfschema_DATA = \
        qsf-object.xsd.xml \
        qsf-map.xsd.xml \
        pilot-qsf-GnuCashInvoice.xml \
	qof-qsf.xml
# qof-qsf.xml is here for testing reasons ONLY!

EXTRA_DIST = \
  $(qsfschema_DATA) \
  qsf-dir.h.in

qsf-dir.h: qsf-dir.h.in
	rm -f $@.tmp
	sed < $< > $@.tmp \
		-e 's:@-QSF_SCHEMA_DIR-@:${QSF_SCHEMA_DIR}:g'
	mv $@.tmp $@

BUILT_SOURCES = qsf-dir.h
