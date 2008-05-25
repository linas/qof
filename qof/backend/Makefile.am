
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

#if USE_ESTRON
#ESTRON=estron
#else
ESTRON=
#endif

if USE_GDABACKEND
GDA=gda
else
GDA=
endif

SUBDIRS = $(XML) $(SQLITE) $(GDA) $(ESTRON)

DIST_SUBDIRS = file sqlite gda $(ESTRON)
