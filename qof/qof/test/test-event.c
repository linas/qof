/***************************************************************************
 *            test-event.c
 *
 *  Sat Feb 11 11:00:02 2006
 *  Copyright  2006  Neil Williams
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor
 *  Boston, MA  02110-1301,  USA 
 */

#include <glib.h>
#include <glib/gprintf.h>
#include "qof.h"
#include "test-engine-stuff.h"
#include "test-stuff.h"

#define OBJ_NAME "somename"
#define OBJ_AMOUNT "anamount"
#define OBJ_DATE "nottoday"
#define OBJ_GUID "unique"
#define OBJ_DISCOUNT "hefty"
#define OBJ_VERSION "early"
#define OBJ_MINOR "tiny"
#define OBJ_ACTIVE "ofcourse"
#define OBJ_FLAG   "tiny_flag"
#define OBJ_EVENT_NAME "test-event-object"
#define OBJ_EVENT_DESC "test object for events"

/* set to TRUE to get QSF XML output
 * requires QSF available (i.e. make install) */
static gboolean debug = FALSE;

/* deliberately make these global to the file to pick up
errors where developers access handlers directly. This practice
will be deprecated in qofevent. */
static guint test, foo;

/* simple object structure */
typedef struct e_obj
{
	QofInstance inst;
	gchar *Name;
	gchar flag;
	gnc_numeric Amount;
	QofTime *date;
	double discount;			/* cheap pun, I know. */
	gboolean active;
	gint32 version;
	gint64 minor;
} event_obj;

static event_obj *
event_create (QofBook * book)
{
	event_obj *e;

	g_return_val_if_fail (book, NULL);
	e = g_new0 (event_obj, 1);
	qof_instance_init (&e->inst, OBJ_EVENT_NAME, book);
	e->date = qof_time_get_current ();
	e->discount = get_random_double ();
	e->active = get_random_boolean ();
	e->version = get_random_int_in_range (1, 10000);
	e->minor = get_random_int_in_range (100000, 99999999);
	e->flag = get_random_character ();
	e->Name = get_random_string ();
	e->Amount = get_random_gnc_numeric ();
	qof_event_gen (&e->inst.entity, QOF_EVENT_CREATE, NULL);
	return e;
}

static void
event_setFlag (event_obj * e, gchar f)
{
	g_return_if_fail (e);
	e->flag = f;
}

static gchar
event_getFlag (event_obj * e)
{
	g_return_val_if_fail (e, 'n');
	return e->flag;
}

static void
event_setMinor (event_obj * e, gint64 h)
{
	g_return_if_fail (e != NULL);
	e->minor = h;
}

static gint64
event_getMinor (event_obj * e)
{
	g_return_val_if_fail (e, 0);
	return e->minor;
}

static void
event_setVersion (event_obj * e, gint32 h)
{
	g_return_if_fail (e);
	e->version = h;
}

static gint32
event_getVersion (event_obj * e)
{
	if (!e)
		return 0;
	return e->version;
}

static void
event_setActive (event_obj * e, gboolean h)
{
	if (!e)
		return;
	e->active = h;
}

static gboolean
event_getActive (event_obj * e)
{
	if (!e)
		return FALSE;
	return e->active;
}

static void
event_setDiscount (event_obj * e, double h)
{
	if (!e)
		return;
	e->discount = h;
}

static double
event_getDiscount (event_obj * e)
{
	if (!e)
		return 0;
	return e->discount;
}

static void
event_setDate (event_obj * e, QofTime *h)
{
	if (!e)
		return;
	e->date = h;
}

static QofTime*
event_getDate (event_obj * e)
{
	if (!e)
		return NULL;
	return e->date;
}

static void
event_setName (event_obj * e, gchar * h)
{
	if (!e || !h)
		return;
	e->Name = strdup (h);
}

static gchar *
event_getName (event_obj * e)
{
	if (!e)
		return NULL;
	return e->Name;
}

static void
event_setAmount (event_obj * e, gnc_numeric h)
{
	if (!e)
		return;
	e->Amount = h;
}

static gnc_numeric
event_getAmount (event_obj * e)
{
	if (!e)
		return gnc_numeric_zero ();
	return e->Amount;
}

static QofObject event_object_def = {
  interface_version:QOF_OBJECT_VERSION,
  e_type:OBJ_EVENT_NAME,
  type_label:OBJ_EVENT_DESC,
  create:(gpointer) event_create,
  book_begin:NULL,
  book_end:NULL,
  is_dirty:NULL,
  mark_clean:NULL,
  foreach:qof_collection_foreach,
  printable:NULL,
  version_cmp:(int (*)(gpointer, gpointer)) qof_instance_version_cmp,
};

static gboolean
event_objRegister (void)
{
	static QofParam params[] = {
		{OBJ_NAME, QOF_TYPE_STRING, (QofAccessFunc) event_getName,
		 (QofSetterFunc) event_setName, NULL},
		{OBJ_AMOUNT, QOF_TYPE_NUMERIC, (QofAccessFunc) event_getAmount,
		 (QofSetterFunc) event_setAmount, NULL},
		{OBJ_DATE, QOF_TYPE_TIME, (QofAccessFunc) event_getDate,
		 (QofSetterFunc) event_setDate, NULL},
		{OBJ_DISCOUNT, QOF_TYPE_DOUBLE, (QofAccessFunc) event_getDiscount,
		 (QofSetterFunc) event_setDiscount, NULL},
		{OBJ_ACTIVE, QOF_TYPE_BOOLEAN, (QofAccessFunc) event_getActive,
		 (QofSetterFunc) event_setActive, NULL},
		{OBJ_VERSION, QOF_TYPE_INT32, (QofAccessFunc) event_getVersion,
		 (QofSetterFunc) event_setVersion, NULL},
		{OBJ_MINOR, QOF_TYPE_INT64, (QofAccessFunc) event_getMinor,
		 (QofSetterFunc) event_setMinor, NULL},
		{OBJ_FLAG, QOF_TYPE_CHAR, (QofAccessFunc) event_getFlag,
		 (QofSetterFunc) event_setFlag, NULL},
		{QOF_PARAM_BOOK, QOF_ID_BOOK, (QofAccessFunc) qof_instance_get_book,
		 NULL, NULL},
		{QOF_PARAM_GUID, QOF_TYPE_GUID, (QofAccessFunc) qof_instance_get_guid,
		 NULL, NULL},
		{NULL, NULL, NULL, NULL, NULL},
	};

	qof_class_register (OBJ_EVENT_NAME, NULL, params);

	return qof_object_register (&event_object_def);
}

typedef struct event_context_s
{
	QofEventId event_type;
	QofEntity *entity_original;
	QofEntity *entity_modified;
	const QofParam *param;
	gboolean destroy_used;
	guint counter;
	guint old_test_id;
	guint old_foo_id;
} event_context;

static void
test_event_handler (QofEntity *ent, QofEventId event_type, 
					gpointer handler_data, 
					gpointer user_data __attribute__ ((unused)))
{
	event_context *context;

	context = (event_context *) handler_data;
	do_test ((ent != NULL), "Null ent in test");
	do_test ((context != NULL), "Null context");
	switch (event_type)
	{
	case QOF_EVENT_NONE:
		{
			break;
		}
	case QOF_EVENT_CREATE:
		{
			break;
		}
	case QOF_EVENT_MODIFY:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity");
			do_test ((context->event_type == QOF_EVENT_MODIFY),
					 "wrong event sent: test (GNC_EVENT_MODIFY)");
			break;
		}
	case QOF_EVENT_DESTROY:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity");
			do_test ((context->event_type == QOF_EVENT_DESTROY),
					 "wrong event sent: test (GNC_EVENT_DESTROY)");
			do_test ((context->destroy_used),
					 "destroy sent without being called");
			/* make sure we can unregister an earlier handler */
			qof_event_unregister_handler (foo);
			break;
		}
	case QOF_EVENT_ADD:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity: test");
			break;
		}
	case QOF_EVENT_REMOVE:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity: test");
			break;
		}
	case QOF_EVENT_ALL:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity: test");
			break;
		}
	}
}

static void
foo_event_handler (QofEntity *ent, QofEventId event_type,
				   gpointer handler_data, 
				   gpointer user_data __attribute__ ((unused)))
{
	event_context *context;

	context = (event_context *) handler_data;
	do_test ((context != NULL), "Null context");
	do_test ((ent != NULL), "Null entity for foo");
	switch (event_type)
	{
	case QOF_EVENT_NONE:
		{
			break;
		}
	case QOF_EVENT_CREATE:
		{
			break;
		}
	case QOF_EVENT_MODIFY:
		{
			break;
		}
	case QOF_EVENT_DESTROY:
		{
			do_test ((context->entity_original != NULL),
					 "No original entity");
			do_test ((context->event_type == QOF_EVENT_DESTROY),
					 "wrong event sent: foo (GNC_EVENT_DESTROY)");
			do_test ((context->destroy_used),
					 "destroy sent without being called");
			/* make sure we can unregister a later handler */
			qof_event_unregister_handler (test);
			break;
		}
	case QOF_EVENT_ADD:
		{
			break;
		}
	case QOF_EVENT_REMOVE:
		{
			break;
		}
	case QOF_EVENT_ALL:
		{
			break;
		}
	}
}

static void
create_data (QofSession * original, event_context * context)
{
	QofBook *start;
	event_obj *e, *e2;

	start = qof_session_get_book (original);
	e = (event_obj *) qof_object_new_instance (OBJ_EVENT_NAME, start);
	do_test ((NULL != &e->inst), "instance init");
	e2 = (event_obj *) qof_object_new_instance (OBJ_EVENT_NAME, start);
	switch (context->counter)
	{
	case 0:
		{						/* empty test */
			do_test ((e != NULL), "empty check");
			break;
		}
	case 1:
		{						/* create a temporary entity, modify it and destroy it */
			event_obj *e1;

			do_test ((context->old_foo_id == foo), "forward foo");
			do_test ((context->old_test_id == test), "forward test");
			context->entity_original = (QofEntity *) e;
			e1 = (event_obj *) qof_object_new_instance (OBJ_EVENT_NAME,
														start);
			do_test ((NULL != &e1->inst), "temporary instance init");
			context->entity_modified = (QofEntity *) e1;
			context->param =
				qof_class_get_parameter (OBJ_EVENT_NAME, OBJ_NAME);
			context->event_type = QOF_EVENT_MODIFY;
			event_setName (e, event_getName (e1));
			qof_event_gen ((QofEntity *) e, QOF_EVENT_MODIFY, NULL);
			context->event_type = QOF_EVENT_DESTROY;
			context->destroy_used = TRUE;
			/* this block unregisters both handlers on DESTROY in turn.
			   Here, foo is unregistered within test */
			qof_event_gen ((QofEntity *) e1, QOF_EVENT_DESTROY, NULL);
			qof_entity_release ((QofEntity *) e1);
			g_free (e1);
			e1 = NULL;
			context->destroy_used = FALSE;
			context->event_type = QOF_EVENT_NONE;
			context->entity_modified = NULL;
			context->param = NULL;
			/* repeat the test in reverse. */
			qof_event_unregister_handler (test);
			test =
				qof_event_register_handler (test_event_handler,
												   context);
			foo =
				qof_event_register_handler (foo_event_handler,
												   context);
			do_test ((context->old_foo_id < foo), "reverse foo");
			do_test ((context->old_test_id < test), "reverse test");
			/* test is unregistered within foo */
			e1 = (event_obj *) qof_object_new_instance (OBJ_EVENT_NAME,
														start);
			context->entity_modified = (QofEntity *) e1;
			context->event_type = QOF_EVENT_DESTROY;
			context->destroy_used = TRUE;
			qof_event_gen ((QofEntity *) e1, QOF_EVENT_DESTROY, NULL);
			qof_entity_release ((QofEntity *) e1);
			g_free (e1);
			e1 = NULL;
			context->destroy_used = FALSE;
			context->event_type = QOF_EVENT_NONE;
			context->entity_original = NULL;
			context->entity_modified = NULL;
			test =
				qof_event_register_handler (test_event_handler,
												   context);
			context->old_foo_id = foo;
			context->old_test_id = test;
			break;
		}
	case 2:
		{	/* create the second test entity */
			context->entity_original = (QofEntity *) e;
			do_test ((NULL != &e2->inst), "second instance init");
			context->entity_modified = (QofEntity *) e2;
			break;
		}
	case 3:
		{	/* destroy the entity e2 */
			context->event_type = QOF_EVENT_DESTROY;
			context->destroy_used = TRUE;
			qof_event_gen ((QofEntity *) e2, QOF_EVENT_DESTROY, NULL);
			qof_entity_release ((QofEntity *) e2);
			g_free (e2);
			e2 = NULL;
			context->destroy_used = FALSE;
			context->event_type = QOF_EVENT_NONE;
			context->entity_modified = NULL;
			break;
		}
	case 4:
		{	/* destroy the original entity e */
			context->event_type = QOF_EVENT_DESTROY;
			context->destroy_used = TRUE;
			qof_event_gen ((QofEntity *) e, QOF_EVENT_DESTROY, NULL);
			qof_entity_release ((QofEntity *) e);
			g_free (e);
			e = NULL;
			context->destroy_used = FALSE;
			context->event_type = QOF_EVENT_NONE;
			context->entity_original = NULL;
			break;
		}
	}
}

int
main (int argc __attribute__ ((unused)), const char *argv[] __attribute__ ((unused)))
{
	QofSession *original;
	event_context context;
	guint count;

	qof_init ();
	event_objRegister ();
	original = qof_session_new ();
	if (debug)
	{
		qof_session_begin (original, QOF_STDOUT, TRUE, FALSE);
	}
	context.event_type = QOF_EVENT_NONE;
	context.entity_original = NULL;
	context.entity_modified = NULL;
	context.destroy_used = FALSE;
	context.param = NULL;
	/* events are unregistered in reverse order, so to test for
	   a bug when unregistering a later module from an earlier one,
	   register the foo module first and unregister it from within
	   a later handler. */
	foo = qof_event_register_handler (foo_event_handler, &context);
	test = qof_event_register_handler (test_event_handler, &context);
	context.old_test_id = test;
	context.old_foo_id = foo;
	for (count = 0; count < 25; count++)
	{
		context.counter = (count % 5);
		create_data (original, &context);
	}
	print_test_results ();
	qof_close ();
	return get_rv();
}
