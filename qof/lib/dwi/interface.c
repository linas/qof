/********************************************************************\
 * interface.c -- Define top-level DWI structure                    *
 * Copyright (C) 2002,2004 Linas Vepstas <linas@linas.org>          *
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
 * Define top-level DWI structure
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */
#include "config.h"

#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <gnome.h>

#include "perr.h"
#include "dui-initdb.h"

#include "action.h"
#include "database.h"
#include "duifilter.h"
#include "interface.h"
#include "report.h"
#include "signal.h"
#include "util.h"
#include "window.h"


struct DuiInterface_s 
{
	/* Application name */
	char * app_name;

	/* Collection of handlers */
	GList *root_windows;
	GList *action_handlers;
	GList *report_handlers;
	GList *databases;

	/* XXX these different hash tables need to be collapsed into
	 * a runtime-extensible system -per-object-system */
	/* Set of key-value pairs */ 
	GHashTable *kvp; 

	/* Set of glib-2.0 gobjects */
	GHashTable *gobs; 

	/* Set of QOF objects */
	GHashTable *qobs; 
};


/* =================================================================== */

DuiInterface * 
dui_interface_new (void)
{
	DuiInterface * dui;

	dui = g_new (DuiInterface,1);

	dui->app_name = NULL;

	dui->action_handlers = NULL;
	dui->report_handlers = NULL;
	dui->root_windows = NULL;
	dui->databases = NULL;

	dui->kvp = g_hash_table_new (g_str_hash, g_str_equal);
	dui->gobs = g_hash_table_new (g_str_hash, g_str_equal);
	dui->qobs = g_hash_table_new (g_str_hash, g_str_equal);

	/* XXX wrong place for this, fixme */
	dui_db_init();

#ifdef HAVE_QOF
gnc_set_log_level (1, GNC_LOG_TRACE);
#endif
	return dui;
}


/* =================================================================== */

#define FREE_STR(str)  if(dui->str) g_free (dui->str); dui->str = NULL;

void 
dui_interface_destroy (DuiInterface *dui)
{
	GList *node;

	if (!dui) return;

	FREE_STR (app_name);

	/* the action and report handlers are deleted when thier 'parent'
	 * window is deleted; we don't need to delete them here. */
	g_list_free (dui->action_handlers);
	g_list_free (dui->report_handlers);

	for (node=dui->root_windows; node; node=node->next)
	{
		DuiWindow *win = node->data;
		dui_window_destroy (win);
	}
	g_list_free (dui->root_windows);

	for (node=dui->databases; node; node=node->next)
	{
		DuiDatabase *db = node->data;
		dui_database_destroy (db);
	}
	g_list_free (dui->databases);

	/* XXX fixme hack alert need to for-each free the keys and values */
	g_hash_table_destroy (dui->kvp);

	/* XXX fixme need to dui_filter_destroy the filter list */
}

/* =================================================================== */

void 
dui_interfaced_set_name (DuiInterface *dui, const char * name)
{
	dui->app_name = g_strdup (name);
}

/* =================================================================== */

void 
dui_interface_add_action (DuiInterface *dui, DuiAction *act)
{
	if (!dui || !act) return;

	dui->action_handlers = g_list_append (dui->action_handlers, act);
}

void 
dui_interface_add_database (DuiInterface *dui, DuiDatabase *db)
{
	if (!dui || !db) return;
	dui->databases = g_list_append (dui->databases, db);
	dui_interface_add_object (dui, dui_database_get_name(db), G_OBJECT(db));
}

void 
dui_interface_add_report (DuiInterface *dui, DuiReport *rpt)
{
	if (!dui || !rpt) return;
	dui->report_handlers = g_list_append (dui->report_handlers, rpt);
}

void 
dui_interface_add_window (DuiInterface *dui, DuiWindow *win)
{
	if (!dui || !win) return;
	dui->root_windows = g_list_append (dui->root_windows, win);
}

/* =================================================================== */

void 
dui_interface_add_object (DuiInterface *dui, const char *instance_name, GObject *gob)
{
	if (!dui || !instance_name) return;
	
	/* We only allow objects; this avoids a check elsewhere */
	if (!G_IS_OBJECT(gob))
	{
		PERR ("The instance \'%s\' is not an object!\n", instance_name);
		return;
	}
	
	/* XXX hack alert -- we need to free the memory associated with 
	 * the instance name  when the interface is deleted */
	g_hash_table_insert (dui->gobs, g_strdup(instance_name), gob);
}

GObject *
dui_interface_find_object_by_name (DuiInterface *dui, const char *key)
{
	if (!dui) return NULL;

	/* intercept pre-defined system keys */
	if (!strncmp (key, "/system/", 8)) 
	{
		return NULL;
	}
	return g_hash_table_lookup (dui->gobs, key);
}

/* =================================================================== */

void
dui_interface_kvp_insert (DuiInterface *dui, const char *key, 
                                             const char *value)
{
	if (!dui || !key || !value) return;

	/* refuse insertion of new system keys */
	if (!strncmp (key, "/system/", 8)) return;

	PINFO ("(key=\'%s\' value=\'%s\')", key, value);
	g_hash_table_insert (dui->kvp, g_strdup(key), g_strdup(value));
}

const char *
dui_interface_kvp_lookup (DuiInterface *dui, const char *key)
{
	if (!dui) return NULL;

	/* intercept pre-defined system keys */
	if (!strncmp (key, "/system/", 8)) 
	{
		key += 8;
		if (!strcmp (key, "datetime"))
		{
			static char date_string [100];
			time_t now = time (0);
			xxxgnc_secs_to_iso8601_buff (now, date_string);
			return date_string;
		}
		return NULL;
	}
	return g_hash_table_lookup (dui->kvp, key);
}

/* =================================================================== */

DuiWindow *
dui_interface_find_window_by_name (DuiInterface *dui, const char * name)
{
	GList *node;

	if (!dui || !name) return NULL;

	for (node=dui->root_windows; node; node=node->next)
	{
		DuiWindow *win = node->data;
		const char *winname = dui_window_get_name (win);
		if (winname && !strcmp(winname, name))
		{
			return win;
		}
	}
	return NULL;
}

/* =================================================================== */

DuiReport *
dui_interface_find_report_by_name (DuiInterface *dui, const char * name)
{
	GList *node;

	if (!dui || !name) return NULL;

	for (node=dui->report_handlers; node; node=node->next)
	{
		DuiReport *rpt = node->data;
		const char *rptname = dui_report_get_name (rpt);
		if (rptname && !strcmp(rptname, name))
		{
			return rpt;
		}
	}
	return NULL;
}

/* =================================================================== */

DuiAction *
dui_interface_find_action_by_name (DuiInterface *dui, const char * name)
{
	GList *node;

	if (!dui || !name) return NULL;

	for (node=dui->action_handlers; node; node=node->next)
	{
		DuiAction *act = node->data;
		const char *actname = dui_action_get_name (act);
		if (actname && !strcmp(actname, name))
		{
			return act;
		}
	}
	return NULL;
}

/* =================================================================== */

DuiDatabase *
dui_interface_find_database_by_name (DuiInterface *dui, const char * name)
{
	GList *node;

	if (!dui || !name) return NULL;

	for (node=dui->databases; node; node=node->next)
	{
		DuiDatabase *db = node->data;
		const char *dbname = dui_database_get_name (db);
		if (dbname && !strcmp(dbname, name))
		{
			return db;
		}
	}
	return NULL;
}

/* =================================================================== */

void 
dui_interface_realize (DuiInterface *dui, int fatal_if_no_main)
{
	GList *node;
	int found_app_window = 0;

	if (!dui) return;

	for (node=dui->action_handlers; node; node=node->next)
	{
		DuiAction *act = node->data;
		dui_action_set_interface (act, dui);
	}
	for (node=dui->report_handlers; node; node=node->next)
	{
		DuiReport *rpt = node->data;
		dui_report_set_interface (rpt, dui);
	}

	for (node=dui->root_windows; node; node=node->next)
	{
		DuiWindow *win = node->data;
		dui_window_resolve (win);
	}

	for (node=dui->root_windows; node; node=node->next)
	{
		DuiWindow *win = node->data;
		int is_app_window = dui_window_is_app_main_window(win);
		if (is_app_window) 
		{
			dui_window_realize (win);
			found_app_window = 1;
		}
	}

	/* Without this test, its just to easy to write a dwi file 
	 * that just hangs there, doing nothing, and you scratch your 
	 * head wondering why ...
	 */
	if (fatal_if_no_main && 0 == found_app_window)
	{
		FATAL ("could not find an application main window!");
	}
}

/* ========================== END OF FILE ============================ */
