/********************************************************************\
 * window.c -- Manage a GUI window                                  *
 * Copyright (C) 2002 Linas Vepstas <linas@linas.org>               *
 * http://dwi.sourceforge.net                                       *
 *                                                                  *
 * This library is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU Lesser General Public       *
 * License as published by the Free Software Foundation; either     *
 * version 2.1 of the License, or (at your option) any later version.
 *                                                                  *
 * This library is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU Lesser General Public License for more details.              *
 *                                                                  *
 * You should have received a copy of the GNU Lesser General Public *
 * License along with this program; if not, contact:                *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/
/*
 * FUNCTION:
 * Manage a data window on the screen.  This window may be
 * and 'input' or an 'output window (or both): acepting user input,
 * windowing results of queries.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */
#include "config.h"

#include <glade/glade.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "perr.h"
#include "util.h"

#include "window.h"
#include "action.h"
#include "interface.h"
#include "report.h"
#include "signal.h"

struct DuiWindow_s
{
	char * name;
	DuiInterface *dui;

	/* glade info */
	char * glade_filepath;
	char * glade_top_widget;

	GladeXML *glxml;
	GtkWidget *widget;       /* top-level window */
	int is_app_main_window;

	/* list of generic signals */
	GList *signals;

	/* lists of data sources/sinks that use this window */
	GList *reports;
	GList *actions;
};

void object_browse (GtkObject *obj);

/* ============================================================ */

DuiWindow * 
dui_window_new (const char * nam, DuiInterface *dwi)
{
	DuiWindow * win;

	if (!nam) return NULL;

	win = g_new (DuiWindow, 1);

	win->name = g_strdup (nam);
	win->dui = dwi;

	win->glade_filepath = NULL;
	win->glade_top_widget = NULL;

	win->glxml = NULL;
	win->widget = NULL;
	win->is_app_main_window = 0;

	win->signals = NULL;
	win->actions = NULL;
	win->reports = NULL;

	dui_interface_add_window (dwi, win);
	return win;
}

void 
dui_window_destroy (DuiWindow *win)
{
	GList *node;

	if (NULL == win) return;

	/* destroy the other guys first; because they may want stuff from us ?? */
	for (node=win->reports; node; node=node->next)
	{
		DuiReport *rpt = node->data;
		dui_report_destroy (rpt);
	}
	g_list_free (win->reports);
	win->reports = NULL;

	for (node=win->actions; node; node=node->next)
	{
		DuiAction *act = node->data;
		dui_action_destroy (act);
	}
	g_list_free (win->actions);
	win->actions = NULL;

	for (node=win->signals; node; node=node->next)
	{
		DuiSignal *sig = node->data;
		dui_signal_destroy (sig);
	}
	g_list_free (win->signals);
	win->signals = NULL;

	if (win->name) g_free (win->name);
	win->name = NULL;

	if (win->glade_filepath) g_free (win->glade_filepath);
	win->glade_filepath = NULL;

	if (win->glade_top_widget) g_free (win->glade_top_widget);
	win->glade_top_widget = NULL;

	win->glxml = NULL;
	win->widget = NULL;
	win->is_app_main_window = 0;

	g_free (win);
}

/* ============================================================ */

const char *
dui_window_get_name (DuiWindow *win)
{
	if (!win) return NULL;
	return (win->name);
}

DuiInterface *
dui_window_get_interface (DuiWindow *win)
{
	if (!win) return NULL;
	return (win->dui);
}

/* ============================================================ */

void
dui_window_set_glade (DuiWindow *win, const char *file, 
                      const char *root, int is_main)
{
	if (!win || !file || !root) return;
	
	win->glade_filepath = g_strdup (file);
	win->glade_top_widget = g_strdup (root);

	win->is_app_main_window = is_main;
}

/* ============================================================ */

void
dui_window_add_action (DuiWindow *win, DuiAction *act)
{
	if (!win || !act) return;
	win->actions = g_list_append (win->actions, act);
}

void
dui_window_add_report (DuiWindow *win, DuiReport *rpt)
{
	if (!win || !rpt) return;
	win->reports = g_list_prepend (win->reports, rpt);
}

void
dui_window_add_signal (DuiWindow *win, DuiSignal *sig)
{
	if (!win || !sig) return;
	win->signals = g_list_append (win->signals, sig);
}

/* ============================================================ */

GtkWidget * 
dui_window_get_widget (DuiWindow *win, const char * widgetname)
{
	GtkWidget *w;
	if (!win || !win->glxml) return NULL;

	/* NULL has special meaning: get the main widget */
	if (!widgetname) return win->widget;

	w = glade_xml_get_widget (win->glxml, widgetname);
	if (NULL == w)
	{
		SYNTAX ("can't find the widget \"%s\" in \"%s\"\n",
			widgetname, win->glade_filepath);
	}
	return w;
}

/* ============================================================ */

static void 
destroy_cb (GtkWidget *widget, DuiWindow *win)
{
	win->glxml = NULL;
	win->widget = NULL;
}

void
dui_window_resolve (DuiWindow *win)
{
	GList *node;
	if (!win) return;

	ENTER ("(win=%p)", win);
	/* Tell the rpoerts, actions about where they will find thier
	 * widgets */
	for (node=win->actions; node; node=node->next)
	{
		DuiAction *act = node->data;
		dui_action_set_window (act, win);
	}
	for (node=win->reports; node; node=node->next)
	{
		DuiReport *rpt = node->data;
		dui_report_set_window (rpt, win);
	}

	LEAVE ("(win=%p)", win);
}

void
dui_window_realize (DuiWindow *win)
{
	GList *node;
	if (!win) return;

	ENTER ("(win=%p)", win);
	/* If already realized, do (almost) nothing */
	/* Note that closing some windows destroys them,
	 * so we need to re-realize when they're gone ... */
	if (win->glxml && win->widget && GTK_IS_WIDGET (win->widget))
	{
		gtk_widget_show (win->widget);
		LEAVE ("(win=%p) { already realized, just showing }", win);
		return;
	}

	win->glxml = glade_xml_new (win->glade_filepath, win->glade_top_widget, NULL);
	if (NULL == win->glxml)
	{
		SYNTAX ("can't open \"%s\"", win->glade_filepath);
		return;
	}

	glade_xml_signal_autoconnect (win->glxml);
	win->widget = glade_xml_get_widget (win->glxml, win->glade_top_widget);
	if (NULL == win->widget)
	{
		SYNTAX ("can't find the widget \"%s\" in \"%s\"\n",
			win->glade_top_widget, win->glade_filepath);
		return;
	}

	gtk_signal_connect (GTK_OBJECT(win->widget), "destroy", 
		GTK_SIGNAL_FUNC(destroy_cb), win);

	/* note bene: the order in which actions and signals are realized affects
	 * the order in which they are delivered.  This is kinda bad, resulting
	 * in ordering effects depending on the organization of the DUI files. 
	 * This is kind-of a bug & needs fixing. Anyways, for right now,
	 * it is important to realize actions before signals, because otherwise
	 * database login won't work :-(
	 */
	for (node=win->actions; node; node=node->next)
	{
		DuiAction *act = node->data;
		dui_action_do_realize (act);
	}

	for (node=win->signals; node; node=node->next)
	{
		DuiSignal *sig = node->data;
		dui_signal_do_realize (sig, win);
	}

	for (node=win->reports; node; node=node->next)
	{
		DuiReport *rpt = node->data;
		dui_report_do_realize (rpt);
	}
	
	gtk_widget_show (win->widget);
	
	LEAVE ("(win=%p)", win);
}

/* ============================================================ */

int
dui_window_is_app_main_window (DuiWindow *win)
{
	if (!win) return 0;

	if (win->is_app_main_window) return 1;
	return 0;
}

/* ============================================================ */

int
dui_window_is_realized (DuiWindow *win)
{
	if (!win) return 0;
	/* Note that closing some windows destroys them,
	 * we can check this by seeing if our pointer is still a widget */
	if (win->glxml && GTK_IS_WIDGET (win->widget)) return 1;
	return 0;
}

/* ============================================================ */

void
dui_window_db_connect (DuiWindow *win)
{
	GList *node;
	if (!win) return;

	for (node=win->actions; node; node=node->next)
	{
		DuiAction *act = node->data;
		dui_action_db_connect (act);
	}
}

/* ============================================================ */
 
#ifdef OLD_BORKEN_CODE
static void
gdata_each (GQuark key_id, gpointer data, gpointer   user_data)
{
	const char * str;
	str = g_quark_to_string (key_id);
	printf ("quark=%d %s\n", key_id, str);
}
#endif /* OLD_BORKEN_CODE */


void
object_browse (GtkObject *obj)
{
#ifdef OLD_BORKEN_CODE
		  XXX fix me someday
	GtkType tipe;
	GtkArg *args;
	int i, nargs;
	guint32 *arg_flags;

	tipe = obj->klass->type;
	printf ("duude the widget type is 0x%x %s\n",  tipe,
						 gtk_type_name(tipe));
	gtk_type_describe_heritage (tipe);

	while (tipe)
	{
		printf ("------------- %s -----------\n", gtk_type_name(tipe));
		args = gtk_object_query_args (tipe, &arg_flags, &nargs);
	
		for (i=0; i<nargs; i++)
		{
			GtkArg *a = &args[i];
			printf ("arg type=%s, arg-name=%s\n", gtk_type_name (a->type), a->name);
		}
	
		gtk_object_getv (obj, nargs, args);

	
		for (i=0; i<nargs; i++)
		{
			GtkArg *a = &args[i];
			const char * tname = gtk_type_name (a->type);
			
			if (!tname) continue;
			if (!strcmp ( tname, "GtkString"))
			{
				printf ("arg arg=%s val=%s\n", a->name, a->d.string_data);
			}
			else if (!strcmp ( tname, "gint"))
			{
				printf ("arg arg=%s val=%d\n", a->name, a->d.int_data);
			}
			else if (!strcmp ( tname, "gboolean"))
			{
				printf ("arg arg=%s val=%d\n", a->name, a->d.bool_data);
			}
		}

		tipe = gtk_type_parent (tipe);
	}
	

	gtk_object_set_data (obj, "junk-a-junk", "duude");
	g_datalist_foreach (&obj->object_data, gdata_each, NULL);
#endif /* OLD_BORKEN_CODE */
}

/* ============================== END OF FILE =================== */
