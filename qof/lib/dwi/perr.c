
/*
 * FILE:
 * perr.c
 *
 * FUNCTION:
 * Error message conduit.
 *
 * HISTORY:
 * Linas Vepstas July 2002
 */

#include "config.h"
#include <gnome.h>


static GString *msgs = NULL;


static int
splat_error_message (gpointer x)
{
	GtkWidget *mbox;
	
#define MAXLEN 900
	if (MAXLEN < msgs->len)
	{
		msgs = g_string_truncate (msgs, MAXLEN);
		msgs = g_string_append (msgs, " ...\n");
	}
	mbox = gnome_message_box_new (msgs->str, GNOME_MESSAGE_BOX_WARNING, 
	                      GNOME_STOCK_BUTTON_OK, NULL);

	gtk_widget_show (mbox);
	
	g_string_free (msgs, TRUE);
	msgs = NULL;
	return 0;
}


void
dui_show_message (const char * dom, GLogLevelFlags log_level, 
                  const char * msg, gpointer x)
{
	if (!msgs) 
	{
		msgs = g_string_new (NULL);
		gtk_idle_add (splat_error_message, NULL);
	}

	msgs = g_string_append (msgs, msg);
	msgs = g_string_append (msgs, "\n");
}

	
/* ============================= END OF FILE =========================== */
