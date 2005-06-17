
/** @file gobj-example.c
 *  @brief Example usage of QOF to query a set of GObjects.
 *  @author Copyright (c) 2004 Linas Vepstas <linas@linas.org>
 *
 * This example program shows how to use QOF to query over a set
 * of GObjects.  In this example, the query is performed over
 * three GtkButtons, each having a different label.  The result
 * of the search is the one GtkButton with the desired label.
 *
 * The use of GtkButton in this example is artificial.  GLib
 * GObjects do not have to be GUI objects, and can be created 
 * for any use desired.  I didn't want to turn this into an 
 * example of how to create a GObject, and so used a pre-existing
 * GObject, the GtkButton.
 */


#include <qof.h>
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
	printf ("\nQuery returned %d results\n", g_list_length (results));

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
