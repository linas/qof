

#include <glib-object.h>


void
qof_gobject_register (GObjectClass *obclass)
{
	GParamSpec **prop_list;
	int n_props;
	prop_list = g_object_class_list_properties (obclass, &n_props);

	printf ("got %d props\n", n_props);
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


