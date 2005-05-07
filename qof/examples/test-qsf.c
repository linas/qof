/***************************************************************************
 *            test-qsf.c
 *
 *  Mon Dec 27 14:00:29 2004
 *  Copyright  2004-2005  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/** @addtogroup Backend
    @{ */
/** @addtogroup QSF QOF Serialisation Format
 */
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include "qsf-xml.h"
#include "qsf-dir.h"
#include "qofinstance-p.h"
#include "qofquery.h"
/** @{ */
/** @file test-qsf.c
    @brief  QSF Object and Map test routines.
    @author Copyright (C) 2004-2005 Neil Williams <linux@codehelp.co.uk>
*/

/** Test file. In time this will grow to be a proper test routine.
	For now, it is a simple console app that implements the QSF test 
	code from libqsf-backend-file.la

A lot of the code is borrowed from test_book_merge - reflecting how
QSF is built on the qof_book_merge codebase.

*/

#define TEST_MODULE_NAME "book_merge_test"
#define TEST_MODULE_DESC "Test Book Merge"
#define OBJ_NAME "somename"
#define OBJ_AMOUNT "anamount"
#define OBJ_DATE "nottoday"
#define OBJ_GUID "unique"
#define OBJ_DISCOUNT "hefty"
#define OBJ_VERSION "early"
#define OBJ_MINOR "tiny"
#define OBJ_ACTIVE "ofcourse"

gboolean myobjRegister (void);

/** \brief simple object structure */
typedef struct obj_s
{
	QofInstance inst;
	char     	*Name;
	gnc_numeric Amount;
	const GUID 	*obj_guid;
	Timespec 	date;
	double 		discount; /* cheap pun, I know. */
	gboolean 	active;
	gint32   	version;
	gint64 		minor;
}myobj;

myobj* obj_create(QofBook*);

/** \section  obvious setter functions 

@{ */
void obj_setName(myobj*,	char*);
void obj_setAmount(myobj*,  gnc_numeric);
void obj_setDate(myobj*,	Timespec h);
void obj_setDiscount(myobj*, double);
void obj_setActive(myobj*,  gboolean);
void obj_setVersion(myobj*, gint32);
void obj_setMinor(myobj*,   gint64);
/** @} */
/** \section obvious getter functions 

@{ */
char*		obj_getName(myobj*);
gnc_numeric obj_getAmount(myobj*);
Timespec   	obj_getDate(myobj*);
double		obj_getDiscount(myobj*);
gboolean	obj_getActive(myobj*);
gint32		obj_getVersion(myobj*);
gint64		obj_getMinor(myobj*);
/** @} */

/** \brief Create a QOF object */
myobj*
obj_create(QofBook *book)
{
	myobj *g;
	QofCollection *coll;
	GList *all;

	g_return_val_if_fail(book, NULL);
	g = g_new(myobj, 1);
	qof_instance_init (&g->inst, TEST_MODULE_NAME, book);
	coll = qof_book_get_collection (book, TEST_MODULE_NAME);
	all = qof_collection_get_data (coll);
	all = g_list_prepend (all, g);
	qof_collection_set_data (coll, all);
	g->date.tv_nsec = 0;
	g->date.tv_sec = 0;
	g->discount = 0;
	g->active = TRUE;
	g->version = 1;
	g->minor = 1;
	return g;
}

void
obj_setMinor(myobj *g, gint64 h)
{
	g_return_if_fail(g != NULL);
	g->minor = h;
}

gint64
obj_getMinor(myobj *g)
{
	g_return_val_if_fail((g != NULL),0);
	return g->minor;
}

void
obj_setVersion(myobj *g, gint32 h)
{
	g_return_if_fail(g != NULL);
	g->version = h;
}

gint32
obj_getVersion(myobj *g)
{
	if(!g) return 0;
	return g->version;
}

void
obj_setActive(myobj *g, gboolean h)
{
	if(!g) return;
	g->active = h;
}

gboolean
obj_getActive(myobj *g)
{
	if(!g) return FALSE;
	return g->active;
}

void
obj_setDiscount(myobj *g, double h)
{
	if(!g) return;
	g->discount = h;
}

double
obj_getDiscount(myobj *g)
{
	if(!g) return 0;
	return g->discount;
}

void
obj_setDate(myobj *g, Timespec h)
{
	if(!g) return;
	g->date = h;
}

Timespec
obj_getDate(myobj *g)
{
	Timespec ts;
	if(!g) return ts;
	ts = g->date;
	return ts;
}

void 
obj_setName(myobj* g, char* h)
{
	if(!g || !h) return;
	g->Name = strdup(h);
}

char*
obj_getName(myobj *g)
{
	if(!g) return NULL;
	return g->Name;
}

void
obj_setAmount(myobj *g, gnc_numeric h)
{
	if(!g) return;
	g->Amount = h;
}

gnc_numeric
obj_getAmount(myobj *g)
{
	if(!g) return double_to_gnc_numeric(0,0,GNC_HOW_DENOM_EXACT);
	return g->Amount;
}

/** \brief Test object QOF definition */
static QofObject obj_object_def = {
  interface_version:     QOF_OBJECT_VERSION,
  e_type:                TEST_MODULE_NAME,
  type_label:            TEST_MODULE_DESC,
  create:                (gpointer)obj_create,
  book_begin:            NULL,
  book_end:              NULL,
  is_dirty:              NULL,
  mark_clean:            NULL,
  foreach:               qof_collection_foreach,
  printable:             NULL,
  version_cmp:           (int (*)(gpointer,gpointer)) qof_instance_version_cmp,
};

/** \brief Test object Parameter registration */
gboolean myobjRegister (void)
{
  static QofParam params[] = {
	{ OBJ_NAME,	    QOF_TYPE_STRING,    (QofAccessFunc)obj_getName,	    (QofSetterFunc)obj_setName	},
	{ OBJ_AMOUNT,   QOF_TYPE_NUMERIC,   (QofAccessFunc)obj_getAmount,   (QofSetterFunc)obj_setAmount	},
	{ OBJ_DATE,	    QOF_TYPE_DATE,      (QofAccessFunc)obj_getDate,	    (QofSetterFunc)obj_setDate	},
	{ OBJ_DISCOUNT, QOF_TYPE_DOUBLE,    (QofAccessFunc)obj_getDiscount, (QofSetterFunc)obj_setDiscount  },
	{ OBJ_ACTIVE,   QOF_TYPE_BOOLEAN,   (QofAccessFunc)obj_getActive,   (QofSetterFunc)obj_setActive	},
	{ OBJ_VERSION,  QOF_TYPE_INT32,	    (QofAccessFunc)obj_getVersion,  (QofSetterFunc)obj_setVersion   },
	{ OBJ_MINOR,    QOF_TYPE_INT64,	    (QofAccessFunc)obj_getMinor,	(QofSetterFunc)obj_setMinor	},
	{ QOF_PARAM_BOOK, QOF_ID_BOOK,	    (QofAccessFunc)qof_instance_get_book, NULL },
	{ QOF_PARAM_GUID, QOF_TYPE_GUID,    (QofAccessFunc)qof_instance_get_guid, NULL },
	{ NULL },
  };

  qof_class_register (TEST_MODULE_NAME, NULL, params);

  return qof_object_register (&obj_object_def);
}


int main (int argc, char **argv)
{
	gboolean result;
	QofBook *tester;
	QofSession *testing;
	double init_value, discount;
	myobj *new_obj;
	Timespec ts;
	gboolean active;
	gint32 version;
	gint64 minor;
	gchar *import_init, *target_init;
	gnc_numeric obj_amount;
	xmlDocPtr qsf_doc;
	gchar *path_buffer;

	gnc_engine_get_string_cache();
	guid_init();
	qof_object_initialize ();
	qof_query_init ();
	qof_book_register ();
	myobjRegister();
 	tester = qof_book_new();

	/* Prepare an empty QSF backend - note lack of qof_session_load call. */
	testing = qof_session_new();
	path_buffer = g_strdup_printf("file:%s/%s", "/tmp", "qsf-test.xml");
	qof_session_begin(testing, path_buffer, TRUE, TRUE);
	tester = qof_session_get_book(testing);

	/* Set some values into the backend QofBook */
	init_value = 1.00;
	result = 0;
	discount = 0.5;
	active = TRUE;
	version = 1;
	minor = 1;
	import_init = "test";
	target_init = "testing";
	qof_date_format_set(QOF_DATE_FORMAT_UK);
	timespecFromTime_t(&ts,time(NULL));
	qsf_doc = NULL;

	new_obj = obj_create(tester);
	obj_setName(new_obj, import_init);
	obj_setAmount(new_obj, obj_amount);
	obj_setActive(new_obj, active);
	obj_setDiscount(new_obj, discount);
	obj_setVersion(new_obj, version);
	obj_setMinor(new_obj, minor);
	obj_setDate(new_obj, ts);

	/* Write out the test data using the QSF backend. */
	qof_session_save(testing, NULL);
	qof_session_end(testing);

	/* Start a new session with an empty book and read the QSF file back in. */
	testing = qof_session_new();
	path_buffer = g_strdup_printf("file:%s/%s", "/tmp", "qsf-test.xml");
	qof_session_begin(testing, path_buffer, TRUE, FALSE);
	qof_session_load(testing, NULL);
	tester = qof_session_get_book(testing);

	return 0;
}
