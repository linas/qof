/********************************************************************\
 * qofgobj.h -- QOF to GLib GObject mapping                         *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef QOF_GOBJ_H
#define QOF_GOBJ_H

/** @addtogroup Engine
    @{ */
/** @file qofgobj.h
    @brief QOF to GLib GObject mapping
    @author Copyright (C) 2004 Linas Vepstas <linas@linas.org>
*/

/** The API defined in this file allows a user to register any
 *  GLib GObject (and any object derived from one, e.g. GTK/Gnome)
 *  with the QOF system so that it becomes searchable.  
 *
 * XXX Only GObject properties are searchable, data and other 
 * hanging off the GObject is not.  Fix this.
 */  

#include <glib-object.h>
#include <qof/qofbook.h>
#include <qof/qofclass.h>

/** Initalize and shut down this subsystem. */
void qof_gobject_init(void);
void qof_gobject_shutdown (void);

/** Register a GObject class with the QOF subsystem.
 *  Doing this will make the properties associated with
 *  this GObject searchable using the QOF subsystem.
 *
 *  The QofType can be any string you desire, although typically
 *  you might want to set it to G_OBJECT_CLASS_NAME() of the 
 *  object class.  Note that this type will become the name of
 *  the "table" that is searched by SQL queries:
 *  e.g. in order to be able to say "SELECT * FROM MyStuff;"
 *  you must first say:
 *   qof_gobject_register ("MyStuff", gobj_class);
 */
void qof_gobject_register (QofType type, GObjectClass *obclass);

/** Register an instance of a GObject with the QOF subsystem.
 *
 *  The QofType can be any string you desire, although typically
 *  you might want to set it to G_OBJECT_CLASS_NAME() of the 
 *  object class.  Note that this type will become the name of
 *  the "table" that is searched by SQL queries:
 *  e.g. in order to be able to say "SELECT * FROM MyStuff;"
 *  you must first say:
 *   qof_gobject_register_instance (book, "MyStuff", obj);
 *
 *  The 'book' argument specifies an anchor point for the collection
 *  of all of the registered instances.  By working with disjoint books,
 *  you can have multiple disjoint searchable sets of objects.
 */

void qof_gobject_register_instance (QofBook *book, QofType, GObject *);

#endif /* QOF_GOBJ_H */
/** @} */

#include <qof/gnc-trace.h>
#include <qof/qof.h>
#include <stdio.h>

static short module = MOD_QUERY;

static gboolean initialized = FALSE;
static GSList *paramList = NULL;
static GSList *classList = NULL;

/* =================================================================== */

#if 0
static gboolean 
clear_table (gpointer key, gpointer value, gpointer user_data)
{
  g_slist_free (value);
  return TRUE;
}
#endif

void 
qof_gobject_init(void)
{
  if (initialized) return;
  initialized = TRUE;
                                                                                
  // gobjectClassTable = g_hash_table_new (g_str_hash, g_str_equal);

	/* Init the other subsystems that we need */
	qof_object_initialize();
	qof_query_init ();
}

void 
qof_gobject_shutdown (void)
{
  if (!initialized) return;
  initialized = FALSE;
                                                                                
	GSList *n;
	for (n=paramList; n; n=n->next) g_free(n->data);
	g_slist_free (paramList);

	for (n=classList; n; n=n->next) g_free(n->data);
	g_slist_free (classList);

#if 0
  // XXX also need to walk over books, and collection and delete
  // the collection get_data instance lists !!
  // without this we have a memory leak !!
  g_hash_table_foreach_remove (gobjectParamTable, clear_table, NULL);
  g_hash_table_destroy (gobjectParamTable);
#endif
}

/* =================================================================== */

#define GOBJECT_TABLE  "GobjectTable"

void 
qof_gobject_register_instance (QofBook *book, QofType type, GObject *gob)
{
	if (!book || !type) return;

	QofCollection *coll = qof_book_get_collection (book, type);

	GSList * instance_list = qof_collection_get_data (coll);
	instance_list = g_slist_prepend (instance_list, gob);
   qof_collection_set_data (coll, instance_list);
}

/* =================================================================== */

static gpointer
qof_gobject_getter (gpointer data, QofParam *getter)
{
	GObject *gob = data;

	GParamSpec *gps = getter->param_userdata;

	if (G_IS_PARAM_SPEC_STRING(gps))
	{
		GValue gval = {G_TYPE_INVALID};
		g_value_init (&gval, G_TYPE_STRING);
		g_object_get_property (gob, getter->param_name, &gval);

      const char * str = g_value_get_string (&gval);
      return (gpointer) str;
   }

	PWARN ("unhandled parameter type %s", G_PARAM_SPEC_TYPE_NAME(gps));
	return NULL;
}

/* =================================================================== */
/* Loop over every instance of the given type in the collection
 * of instances that we have on hand.
 */
static void
qof_gobject_foreach (QofCollection *coll, QofEntityForeachCB cb, gpointer ud)
{
   GSList *n;
	n = qof_collection_get_data (coll);
   for (; n; n=n->next)
   {
      cb (n->data, ud);
   }
}
                                                                                
/* =================================================================== */

void
qof_gobject_register (QofType e_type, GObjectClass *obclass)
{

	/* Get the GObject properties, convert to QOF properties */
	GParamSpec **prop_list;
	int n_props;
	prop_list = g_object_class_list_properties (obclass, &n_props);

	QofParam * qof_param_list = g_new0 (QofParam, n_props);
	paramList = g_slist_prepend (paramList, qof_param_list);

	PINFO ("object %s has %d props", e_type, n_props);
	int i, j=0;
	for (i=0; i<n_props; i++)
	{
		GParamSpec *gparam = prop_list[i];
		QofParam *qpar = &qof_param_list[j];

		PINFO ("param %d %s is type %s", 
		      i, gparam->name, G_PARAM_SPEC_TYPE_NAME(gparam));

		qpar->param_name = g_param_spec_get_name (gparam);
		qpar->param_getfcn = qof_gobject_getter;
		qpar->param_setfcn = NULL;
		qpar->param_userdata = gparam;
		if ((G_IS_PARAM_SPEC_INT(gparam))  ||
		    (G_IS_PARAM_SPEC_UINT(gparam)) ||
		    (G_IS_PARAM_SPEC_ENUM(gparam)) ||
		    (G_IS_PARAM_SPEC_FLAGS(gparam)))
		{
			qpar->param_type = QOF_TYPE_INT32;
			j++;
		} 
		else
		if ((G_IS_PARAM_SPEC_INT64(gparam)) ||
		    (G_IS_PARAM_SPEC_UINT64(gparam)))
		{
			qpar->param_type = QOF_TYPE_INT64;
			j++;
		} 
		else
		if (G_IS_PARAM_SPEC_BOOLEAN(gparam))
		{
			qpar->param_type = QOF_TYPE_BOOLEAN;
			j++;
		} 
		else
		if (G_IS_PARAM_SPEC_STRING(gparam))
		{
			qpar->param_type = QOF_TYPE_STRING;
			j++;
		} 
		else
		if ((G_IS_PARAM_SPEC_POINTER(gparam)) ||
		    (G_IS_PARAM_SPEC_OBJECT(gparam)))
		{
			/* No-op, silently ignore.  Someday we should handle this ...  */
		} 
		else
		if ((G_IS_PARAM_SPEC_FLOAT(gparam)) ||
		    (G_IS_PARAM_SPEC_DOUBLE(gparam)))
		{
			qpar->param_type = QOF_TYPE_DOUBLE;
			j++;
		} 
		else
		if (G_IS_PARAM_SPEC_CHAR(gparam))
		{
			qpar->param_type = QOF_TYPE_CHAR;
			j++;
		} 
		else
		{
			PWARN ("Unknown/unhandled parameter type %s on %s:%s\n", 
			    G_PARAM_SPEC_TYPE_NAME(gparam), e_type, qpar->param_name);
		}
	}

	/* NULL-terminated list! */
	qof_param_list[j].param_type = NULL;

   qof_class_register (e_type, NULL, qof_param_list);

	/* ------------------------------------------------------ */
   /* Now do the class itself */
	QofObject *class_def = g_new0 (QofObject, 1);
	classList = g_slist_prepend (classList, class_def);

	class_def->interface_version = QOF_OBJECT_VERSION;
	class_def->e_type = e_type;
	/* We could let the user specify a "nick" here, but
	 * the actual class name seems reasonable, e.g. for debugging. */
	class_def->type_label = G_OBJECT_CLASS_NAME (obclass);
	class_def->book_begin = NULL;
	class_def->book_end = NULL;
	class_def->is_dirty = NULL;
	class_def->mark_clean = NULL;
	class_def->foreach = qof_gobject_foreach;
	class_def->printable = NULL;
 
   qof_object_register (class_def);
}

/* ======================= END OF FILE ================================ */

#include <qof/qof.h>
#include <gtk/gtk.h>

int
main(int argc, char *argv[])
{
	g_type_init ();
	gtk_init (&argc, &argv);
	qof_gobject_init ();

	/* Register a new searchable type */
	GtkWidget *w = gtk_button_new_with_label ("Howdy Doody");
	GtkButtonClass * wc = GTK_BUTTON_GET_CLASS(w);
	GObjectClass *goc = G_OBJECT_CLASS(wc);

	qof_gobject_register ("MyGtkButton", goc);

	/* Add a number of instances to the collection */
   QofBook *book =  qof_book_new();
	qof_gobject_register_instance (book, "MyGtkButton", G_OBJECT(w));

	w = gtk_button_new_with_label ("dorf");
	qof_gobject_register_instance (book, "MyGtkButton", G_OBJECT(w));

	w = gtk_button_new_with_label ("zinger");
	qof_gobject_register_instance (book, "MyGtkButton", G_OBJECT(w));

	printf ("\nCreated three objects and registered them\n\n");

   /* Create a new query, run that query */
	QofSqlQuery *q;
   q =  qof_sql_query_new ();
 
   /* Set the book to be searched */
   qof_sql_query_set_book(q, book);

	char * qstr = "SELECT * FROM MyGtkButton WHERE (label = 'dorf')";
	printf ("Going to perform the query %s\n", qstr); 

	GList *results = qof_sql_query_run (q, qstr);
	printf ("Query returned %d results\n", g_list_length (results));

	GList *n;
	for (n=results; n; n=n->next)
	{
		GtkWidget *qw = n->data;
		if (GTK_IS_BUTTON(qw))
		{
			GtkButton *bw = GTK_BUTTON (qw);
			printf ("Found a button whose label is %s\n", gtk_button_get_label(bw));
		}
	}

	qof_gobject_shutdown ();
	return 0;
}

/* =================== END OF FILE ===================== */
