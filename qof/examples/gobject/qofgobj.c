
#if 0
* Provide infrastructure to register my object with QOF */
 
static QofObject myobj_object_def =
{
   interface_version: QOF_OBJECT_VERSION,
   e_type:            MYOBJ_ID,
   type_label:        "My Blinking Object",
   book_begin:        NULL,
   book_end:          NULL,
   is_dirty:          NULL,
   mark_clean:        NULL,
   foreach:           my_obj_foreach,
   printable:         NULL,
};
 
gboolean myObjRegister (void)
{
   /* Associate an ASCII name to each getter, as well as the return type
*/
   static QofParam params[] = {
     { MYOBJ_A,     QOF_TYPE_INT32, (QofAccessFunc)my_obj_get_a, NULL },
     { MYOBJ_B,     QOF_TYPE_INT32, (QofAccessFunc)my_obj_get_b, NULL },
     { MYOBJ_MEMO,  QOF_TYPE_STRING, (QofAccessFunc)my_obj_get_memo,
NULL },
     { NULL },
   };
 
   qof_class_register (MYOBJ_ID, (QofSortFunc)my_obj_order, params);
   return qof_object_register (&myobj_object_def);
}
                                                                                
#endif

#include <glib-object.h>
#include <qof/qof.h>
#include <stdio.h>

static gpointer
gobject_int_getter (gpointer data)
{
	GObject *gob = data;

	// damn I need lambda .... 
	return NULL;
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

	// XXX memory leak 
	QofParam * qof_param_list = g_new0 (QofParam, n_props);

	printf ("got %d props\n", n_props);
	int i, j=0;
	for (i=0; i<n_props; i++)
	{
		GParamSpec *gparam = prop_list[i];
		QofParam *qpar = &qof_param_list[j];

		printf ("%d %s\n", i, gparam->name);

		qpar->param_name = g_param_spec_get_name (gparam);
		if (G_IS_PARAM_SPEC_INT(gparam))
		{
			qpar->param_type = QOF_TYPE_INT32;
			qpar->param_getfcn = gobject_int_getter;
			qpar->param_setfcn = NULL;
printf ("its an int!! %s \n", qpar->param_name);
			j++;
		}
	}

   // qof_class_register (qof_e_type, (QofSortFunc)my_obj_order, qof_param_list);
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

	qof_gobject_register (goc);

	return 0;
}


/* =================== END OF FILE ===================== */
