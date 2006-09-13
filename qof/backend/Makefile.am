
if EMBEDDED
XML=
SQLITE=sqlite
else
if USE_SQLITE
SQLITE=sqlite
else
SQLITE=
endif
XML=file
endif

if USE_LIBGDA
GDA=gda
else
GDA=
endif

SUBDIRS = $(XML) $(SQLITE) $(GDA)

DIST_SUBDIRS = dwi file sqlite gda
