/********************************************************************\
 * signal.c -- Handle a gtk signal                                  *
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
 * Handle a gtk signal
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */
#include "config.h"

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "perr.h"

#include "interface.h"
#include "signal.h"
#include "window.h"

typedef void  (*DuiCallback) (GObject *, gpointer);

struct DuiSignal_s
{
	char * signalname;
	char * widgetname;
	char * actionname;
	GObject *gobj;     /* when this object is activated, the action will run */
	guint  signal_id;   /* the gtk_signal id */

	DuiCallback sigfunc;
	gpointer user_data;
};


/* ============================================================ */

DuiSignal * 
dui_signal_new (const char * wig, const char * signame, const char *act)
{
	DuiSignal *sig;

	if (!signame || !wig || !act) return NULL;

	sig = g_new (DuiSignal, 1);
	sig->widgetname = g_strdup (wig);
	sig->signalname = g_strdup (signame);
	sig->actionname = g_strdup (act);
	sig->gobj = NULL;
	sig->signal_id = -1;
	sig->sigfunc = NULL;
	sig->user_data = NULL;

	return sig;
}

/* ============================================================ */
/* XXX should probably be using GClosure in here */

void
dui_signal_destroy (DuiSignal *sig)
{
	if (!sig) return;
 	if (sig->gobj && G_IS_OBJECT(sig->gobj) )
	{
		g_signal_handler_disconnect (sig->gobj, sig->signal_id);
	}
	g_free (sig->widgetname);
	g_free (sig->signalname);
	g_free (sig->actionname);

	sig->widgetname = NULL;
	sig->signalname = NULL;
	sig->actionname = NULL;

	sig->gobj = NULL;
	sig->signal_id = -1;
	sig->sigfunc = NULL;
	sig->user_data = NULL;

	g_free (sig);
}

/* ============================================================ */

const char *
dui_signal_get_widgetname (DuiSignal *sig)
{
	if (!sig) return NULL;
	return sig->widgetname;
}

/* ============================================================ */
/* xxx FIXME hack alert -- should use GtkSignalQuery to determine
 * the proper shim type.
 */

static void
sig_shim_vi (GObject *obj, gint x, DuiSignal *sig)
{
	(sig->sigfunc) (obj, sig->user_data);
}

static void
sig_shim_viip (GObject *obj, gint x, gint y, gpointer z, DuiSignal *sig)
{
	(sig->sigfunc) (obj, sig->user_data);
}

void
dui_signal_connect (DuiSignal *sig, GObject *obj, 
                    GCallback sigfunc, gpointer user_data)
{
	if (!sig || !obj || !sigfunc) return;

	sig->gobj = obj;
	sig->sigfunc = (DuiCallback) sigfunc;
	sig->user_data = user_data;
	
	/* shim because user_data is not second arg */
	if ((GTK_IS_CLIST(obj) || GTK_IS_CTREE(obj)) &&
	      ((0 == strcmp (sig->signalname, "select_row"))
	    || (0 == strcmp (sig->signalname, "unselect_row"))))
	{
		sig->signal_id = g_signal_connect_after(obj, 
	                   sig->signalname, G_CALLBACK(sig_shim_viip), sig);
	}
	else
	if ((GTK_IS_CLIST(obj) || GTK_IS_CTREE(obj)) &&
	      ((0 == strcmp (sig->signalname, "fake_unselect_all"))))
	{
		sig->signal_id = g_signal_connect_after(obj, 
	                   sig->signalname, G_CALLBACK(sig_shim_vi), sig);
	}
	else
	{
		sig->signal_id = g_signal_connect_after(obj, 
	                    sig->signalname, sigfunc, user_data);
	}
}

void
dui_signal_connect_in_window (DuiSignal *sig, DuiWindow *win, 
                    GCallback sigfunc, gpointer user_data)
{
	GtkWidget *w;
	w = dui_window_get_widget (win, sig->widgetname);

	/* Might not be a widget if the root window doesn't exist */
	if (!w) return;
	dui_signal_connect (sig, G_OBJECT(w), sigfunc, user_data);
}

/* ============================================================ */

static void
close_window (GObject *w, gpointer user_data)
{
	DuiWindow *win = user_data;
	gtk_widget_hide (dui_window_get_widget(win, NULL));
}

static void
main_quit (GObject *w, gpointer user_data)
{
	gtk_main_quit();
}

static void
db_connect (GObject *w, gpointer user_data)
{
	DuiWindow *win = user_data;
	dui_window_db_connect (win);
}

/* ============================================================ */

void
dui_signal_do_realize (DuiSignal *sig, DuiWindow *dwin)
{
	if (!sig || !dwin) return;
	ENTER ("(sig=%p)", sig);

	if (!strcmp (sig->actionname, "close_window"))
	{
		sig->gobj = G_OBJECT (dui_window_get_widget(dwin, sig->widgetname));
		sig->sigfunc = close_window;
		sig->user_data = dwin;
	
		sig->signal_id = g_signal_connect(sig->gobj, sig->signalname, 
			G_CALLBACK(close_window), dwin);
	}
	else if (!strcmp (sig->actionname, "main_quit"))
	{
		sig->gobj = G_OBJECT (dui_window_get_widget(dwin, sig->widgetname));
		sig->sigfunc = main_quit;
		sig->user_data = dwin;
	
		sig->signal_id = g_signal_connect(sig->gobj, sig->signalname, 
			G_CALLBACK(main_quit), dwin);
	}
	else if (!strcmp (sig->actionname, "db_connect"))
	{
		sig->gobj = G_OBJECT (dui_window_get_widget(dwin, sig->widgetname));
		sig->sigfunc = db_connect;
		sig->user_data = dwin;
	
		sig->signal_id = g_signal_connect_after(sig->gobj, sig->signalname, 
			G_CALLBACK(db_connect), dwin);
	}
	else if (!strcmp (sig->actionname, "submit_form"))
	{
		/* no-op */
	}
	else
	{
		PERR ("Unknown signal type \'%s\'", sig->actionname);
	}
	LEAVE ("(sig=%p)", sig);
}

/* ============================== END OF FILE =================== */
