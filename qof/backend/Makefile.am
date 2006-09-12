
if EMBEDDED
XML=
SQLITE=sqlite
else
SQLITE=
XML=file
endif

if QSQANDQSF
SQLITE=sqlite
endif

if USE_LIBGDA
GDA=gda
else
GDA=
endif

SUBDIRS = $(XML) $(SQLITE) $(GDA)

DIST_SUBDIRS = dwi file sqlite gda
