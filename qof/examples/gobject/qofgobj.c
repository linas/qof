
#include <glib-object.h>
#include <qof/qof.h>
#include <stdio.h>

void 
qof_gobject_register_instance (GType type, GObject *gob)
{
}

static gpointer
qof_gobject_getter (gpointer data, QofParam *getter)
{
	GObject *gob = data;

printf ("duude trying to get type %s\n", getter->param_type);
	// damn I need lambda .... 
	return NULL;
}

/* Loop over every instance of th
xxxxxxxxxxxxxxxxxxxxxx
MyObj, and apply the callback to it.
 * This routine must be defined for queries to be possible. */
static void
qof_gobject_foreach (QofCollection *coll, QofEntityForeachCB cb, gpointer ud)
{
	printf ("duude foreach caled \n");
}
                                                                                


void
qof_gobject_register (GObjectClass *obclass)
{
	/* Get the "type" as a string */
	const char * qof_e_type =  G_OBJECT_CLASS_NAME (obclass);

	printf ("object is %s\n", qof_e_type);

	/* Get the GObject properties, convert to QOF properties */
	GParamSpec **prop_list;
	int n_props;
	prop_list = g_object_class_list_properties (obclass, &n_props);

	// XXX memory leak  need to free this someday
	QofParam * qof_param_list = g_new0 (QofParam, n_props);

	printf ("got %d props\n", n_props);
	int i, j=0;
	for (i=0; i<n_props; i++)
	{
		GParamSpec *gparam = prop_list[i];
		QofParam *qpar = &qof_param_list[j];

		printf ("%d %s\n", i, gparam->name);

		qpar->param_name = g_param_spec_get_name (gparam);
		qpar->param_getfcn = qof_gobject_getter;
		qpar->param_setfcn = NULL;
		if (G_IS_PARAM_SPEC_INT(gparam))
		{
			qpar->param_type = QOF_TYPE_INT32;
printf ("its an int!! %s \n", qpar->param_name);
			j++;
		}
	}
	/* NULL-terminaed list !! */
	qof_param_list[j].param_type = NULL;

   qof_class_register (qof_e_type, NULL, qof_param_list);

	/* ------------------------------------------------------ */
   /* Now do the class itself */
	// XXX memory leak,////  fixme
	QofObject *class_def = g_new0 (QofObject, 1);

	class_def->interface_version = QOF_OBJECT_VERSION;
	class_def->e_type = qof_e_type;
	class_def->type_label = qof_e_type; // XXX we want nickname, actually
	class_def->book_begin = NULL;
	class_def->book_end = NULL;
	class_def->is_dirty = NULL;
	class_def->mark_clean = NULL;
	class_def->foreach = qof_gobject_foreach;
	class_def->printable = NULL;
 
   qof_object_register (&myobj_object_def);
}

#include <gtk/gtk.h>

int
main(int argc, char *argv[])
{
	g_type_init ();

	gtk_init (&argc, &argv);

	GtkWidget *w = gtk_button_new_with_label ("Howdy Doody");

	GtkButtonClass * wc = GTK_BUTTON_GET_CLASS(w);

	GObjectClass *goc = G_OBJECT_CLASS(wc);

	qof_query_init ();
	qof_gobject_register (goc);

	return 0;
}


/* =================== END OF FILE ===================== */
