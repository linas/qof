

all: qofgobj


INCLUDES= `pkg-config --cflags glib-2.0 gobject-2.0 gtk+-2.0`
LIBS= `pkg-config --libs glib-2.0 gobject-2.0 gtk+-2.0`

qofgobj: qofgobj.o

.o:
	cc -o $* $^ ${LIBS}
 
.c.o:
	cc -c -g -Wall $< ${CFLAGS} ${INCLUDES}
 


