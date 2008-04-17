
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

if USE_DWI
DWI=dwi
else
DWI=
endif

if USE_GDABACKEND
GDA=gda
else
GDA=
endif

SUBDIRS = $(XML) $(SQLITE) $(GDA) $(DWI)

DIST_SUBDIRS = file sqlite gda dwi
