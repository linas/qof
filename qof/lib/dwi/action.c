/********************************************************************\
 * action.c --  Handler for a button click.                         *
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
 * Handler for a button click.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#include "config.h"

#include <string.h>

#include <glib.h>

#include "action.h"
#include "duifield.h"
#include "duifield-gobj.h"
#include "duifield-gtk.h"
#include "duifield-hash.h"
#include "duifield-qof.h"
#include "duifieldmap.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "duitxnquery.h"
#include "perr.h"
#include "report.h"
#include "signal.h"
#include "window.h"

typedef struct DuiCheck_s DuiCheck;
typedef struct DuiRefresh_s DuiRefresh;

struct DuiAction_s
{
	char * name;                 /**< Our name */

	/** SQL database object.  */
	DuiTxnQuery *query;          /**< Default query, if no other specified */
	char * database_name;        /**< Name of database to use in query */

	/* List of non-table targets (the target field is not an sql table) */
	GList *non_table_targets;

	/* The report that will report the results of the query */
	char * report_name;
	DuiReport * report;

	/** List of gtk signals that can trigger this action. */
	GList *signals;

	/** List of checks that must be performed first (and pass)
	 * before the form is allowed to proceed.
	 */
	GList *check_list;

	/** List of windows that need to be refreshed by this action */
	GList *refresh_list;

	/** Name resolution infrastructure. Please note similarity to the 
	 * DuiReport structure. This is not an accident.
	 */
	DuiResolver  *resolver;
	DuiWindow    *window;     /**< Window holding src/target widgets */
	DuiInterface *interface;  /**< global anchor */
	QofBook      *book;       /**< Book holding objects */
	gboolean is_finalized;
};

struct DuiRefresh_s
{
	char * reportname;
	DuiReport *report;
};

struct DuiCheck_s
{
	char * actionname;
	DuiAction *action;
};

/* =================================================================== */

DuiAction * 
dui_action_new (const char * name)
{
	DuiAction *act;

	act = g_new0 (DuiAction, 1);
	act->name = g_strdup (name);

	act->signals = NULL;

	act->query = NULL;
	act->database_name = NULL;
	act->non_table_targets = NULL;

	act->report_name = NULL;
	act->report = NULL;
	act->check_list = NULL;
	act->refresh_list = NULL;
	
	act->resolver = dui_resolver_new();
	act->window = NULL;
	act->interface = NULL;
	act->book = NULL;
	act->is_finalized = FALSE;

	return act;
}

/* =================================================================== */

void 
dui_action_destroy (DuiAction * act)
{
	GList *node;
	if (!act) return;

	act->window = NULL;
	act->interface = NULL;
	if (act->name) g_free (act->name);
	act->name = NULL;
	if (act->database_name) g_free (act->database_name);
	act->database_name = NULL;

	dui_txnquery_destroy (act->query);

	/* Remove terms */
	for (node=act->non_table_targets; node; node=node->next)
	{
		DuiFieldMap *fm = node->data;
		dui_field_map_destroy (fm);
	}
	g_list_free (act->non_table_targets);
	act->non_table_targets = NULL;

	g_free(act->report_name);
	act->report_name = NULL;
	act->report = NULL;

	/* the signals themselves will be reaped by window */
	/* XXX probably not ... */
	g_list_free (act->signals);
	act->signals = NULL;

	for (node=act->check_list; node; node=node->next)
	{
		DuiCheck *chk = node->data;
		g_free (chk->actionname);
		g_free (chk);
	}
	for (node=act->refresh_list; node; node=node->next)
	{
		DuiRefresh *rsh = node->data;
		g_free (rsh->reportname);
		g_free (rsh);
	}
	g_list_free (act->refresh_list);

	dui_resolver_destroy (act->resolver);
	act->window = NULL;
	act->interface = NULL;
	act->book = NULL;
	g_free (act);
}

/* =================================================================== */

const char * 
dui_action_get_name (DuiAction *act)
{
	if (!act) return NULL;
	if (act->name) return (act->name);
	return dui_window_get_name (act->window);
}

/* =================================================================== */

void 
dui_action_set_report (DuiAction * act, const char * rptname)
{
	if (!act) return;
	if (act->report_name) g_free (act->report_name);
	act->report_name = g_strdup (rptname);
}

const char * 
dui_action_get_report (DuiAction * act)
{
	if (!act) return NULL;
	return act->report_name;
}

/* =================================================================== */

void 
dui_action_add_chain (DuiAction *act, const char * actname)
{
	DuiCheck *chk;
	if (!act) return;
	
	chk = g_new (DuiCheck, 1);
	chk->actionname = g_strdup (actname);
	chk->action = NULL;

	act->check_list = g_list_prepend (act->check_list, chk);
}

/* =================================================================== */

void 
dui_action_add_refresh (DuiAction *act, const char * name)
{
	DuiRefresh *rsh;
	if (!act) return;
	
	rsh = g_new (DuiRefresh, 1);
	rsh->reportname = g_strdup (name);
	rsh->report = NULL;

	act->refresh_list = g_list_prepend (act->refresh_list, rsh);
}

/* =================================================================== */

void 
dui_action_set_database_name (DuiAction * act, const char *dbname)
{
	if (!act || !dbname) return;
	if (act->database_name) g_free (act->database_name);
	act->database_name = g_strdup (dbname);
}

/* =================================================================== */

void 
dui_action_add_signal (DuiAction *act, const char *objectname, const char *signalname)
{
	DuiSignal *sig;

	if (!act || !objectname || !signalname) return;
	
	sig = dui_signal_new (objectname, signalname, "submit_form");
	act->signals = g_list_append (act->signals, sig);
}
			
/* =================================================================== */

void 
dui_action_add_row (DuiAction * act, DuiTxnQuery *qry)
{
	if (!act || !qry) return;

	dui_txnquery_set_resolver (qry, act->resolver);

	/* XXX we should do something if this is called twice ... */
	if (act->query)
	{
		PERR ("There already is a query in this action");
	}
	act->query = qry;
}

DuiTxnQuery *
dui_action_get_query (DuiAction * act)
{
	if (!act) return NULL;
	return act->query;
}

void 
dui_action_add_term (DuiAction * act, DuiFieldMap *fm)
{
	if (!act || !fm) return;

	if (DUI_FIELD_IS_TYPE(&fm->target, DUI_FIELD_SQL) ||
	    DUI_FIELD_IS_TYPE(&fm->target, DUI_FIELD_WHERE) ||
	    DUI_FIELD_IS_TYPE(&fm->target, DUI_FIELD_CONST))
	{
		if (NULL == act->query)
		{
			DuiTxnQuery *qry = dui_txnquery_new();
			dui_action_add_row (act, qry);
			act->query = qry;
		}
		dui_txnquery_add_term (act->query, fm);
	}
	else
	{
		dui_resolver_add_field (act->resolver, &fm->source);
		dui_resolver_add_field (act->resolver, &fm->target);
		act->non_table_targets = g_list_append (act->non_table_targets, fm);
	}
}

/* =================================================================== */
/* "final_setup" resolves window names, filter names, etc, without
 * actually touching the GUI in any way.
 */

static void
resolve_widget_again (DuiAction *act)
{
	GList *node;
	for (node=act->non_table_targets; node; node=node->next)
	{
		DuiFieldMap *fm = node->data;
		dui_field_map_resolve (fm);
	}
	dui_resolver_resolve_widgets (act->resolver, act->window);
}

static void
final_setup (DuiAction *act)
{
	GList *node;

	if (act->is_finalized) return;
	act->is_finalized = TRUE;
	ENTER ("(act=%p)", act);
	
	/* Locate the reports the action will use, and plug them in. */
	DuiInterface *iface = act->interface;
	act->report = dui_interface_find_report_by_name (iface, act->report_name);
	if (act->report_name && !act->report) 
	{
		const char * tmp = dui_action_get_name(act);
		SYNTAX ("In form \'%s\', can't find the report \'%s\'", 
		                tmp, act->report_name);
	}

	/* Do the same thing for the check list */
	for (node=act->check_list; node; node=node->next)
	{
		DuiCheck *chk = node->data;
		chk->action = dui_interface_find_action_by_name (iface, chk->actionname);

		if (chk->actionname && !chk->action) 
		{
			const char * tmp = dui_action_get_name(act);
			SYNTAX ("In form \'%s\', can't find the chain form \'%s\'", 
			                tmp, chk->actionname);
		}
	}
	
	/* Do the same thing for the refresh list */
	for (node=act->refresh_list; node; node=node->next)
	{
		DuiRefresh *rsh = node->data;
		rsh->report = dui_interface_find_report_by_name (iface, rsh->reportname);

		if (rsh->reportname && !rsh->report) 
		{
			const char * tmp = dui_action_get_name(act);
			SYNTAX ("In form \'%s\', can't find the refresh report \'%s\'", 
			                tmp, rsh->reportname);
		}
	}
	
	for (node=act->non_table_targets; node; node=node->next)
	{
		DuiFieldMap *fm = node->data;
		dui_field_map_resolve (fm);
	}
	dui_report_do_realize (act->report);

	/* Convert widget names into pointers to actual widgets */
	dui_resolver_resolve_widgets (act->resolver, act->window);

	/* Pull global stuff out of global anchor point */
	dui_resolver_resolve_gobj (act->resolver, act->interface);
	dui_resolver_resolve_hash (act->resolver, act->interface);
	dui_resolver_resolve_qof (act->resolver, act->book);


	/* Resolve the databases too. */
	DuiDatabase *db;
	db = dui_interface_find_database_by_name (act->interface, act->database_name);
	dui_txnquery_set_database (act->query, db);
	dui_txnquery_do_realize (act->query);

	LEAVE ("(act=%p)", act);
}

static void
dui_action_signal_handler (GObject *obj, DuiAction *act)
{
	dui_action_run (act);
}

/* ============================================================ */

void
dui_action_set_window (DuiAction *act, DuiWindow *dwin)
{
	if (!act) return;
	act->window = dwin;
}

void
dui_action_set_interface (DuiAction *act, DuiInterface *iface)
{
	if (!act) return;
	act->interface = iface;
}

void
dui_action_set_book (DuiAction *act, QofBook *book)
{
	if (!act) return;
	act->book = book;
}

void
dui_action_do_realize (DuiAction *act)
{
	GList *node;
	if (!act) return;

	ENTER ("(act=%p)", act);

	final_setup (act);

	DuiWindow *dwin = act->window;
	for (node=act->signals; node; node=node->next)
	{
		DuiSignal *sig = node->data;

		/* XXX this looks wrong to me  ... looks like a double-connect to me */
		dui_signal_connect_in_window (sig, dwin, 
		         G_CALLBACK(dui_action_signal_handler), act);
		dui_signal_do_realize (sig, dwin);
	}

	LEAVE ("(act=%p)", act);
}

/* =================================================================== */

void 
dui_action_db_connect (DuiAction *act)
{
	if (!act) return;
	dui_txnquery_connect (act->query);
}

/* =================================================================== */

int
dui_action_run (DuiAction *act)
{
	DuiDBRecordSet *recs = NULL;
	GList *node;
	int rc = 0;

	if (!act) return 0;
	ENTER ("(act=%p \'%s\')", act, dui_action_get_name (act));
	final_setup (act);

	/* We need to resolve the widgets again, because the action window
	 * may have been closed. When it is re-realized, there will be all
	 * new widgets for the thing.  It would probably be more efficient
	 * (and more complex) to catch the window-close signal. Oh well. */
	resolve_widget_again (act);

	/* Loop over chained actions; quit if one of them handled things */
	for (node=act->check_list; node; node=node->next)
	{
		DuiCheck *chk = node->data;
		rc = dui_action_run (chk->action);
		if (rc) return rc;
	}

	/* Loop over list of field maps that don't involve any SQL tables,
	 * and move the data from the source to the target. */
	for (node = act->non_table_targets; node; node=node->next)
	{
		DuiFieldMap *fm = node->data;
		dui_field_map_transfer_data (fm);
	}
	
	recs = dui_txnquery_run (act->query);

	/* Now report the results. */
	dui_report_set_last_action (act->report, act);
	rc = dui_report_show_data (act->report, recs);

	/* Don't need it no more */
	dui_recordset_free (recs);

	/* Refresh all windows that show data modified by this query */
	for (node=act->refresh_list; node; node=node->next)
	{
		DuiRefresh *rsh = node->data;
		dui_report_refresh (rsh->report);
	}
	LEAVE ("(act=%p \'%s\') rc=%d", act, dui_action_get_name (act), rc);
	return rc;
}

/* =================================================================== */

void
dui_action_rerun_last_query (DuiAction *act)
{
	DuiDBRecordSet *recs = NULL;

	if (!act) return;

	ENTER ("(act=%p \'%s\')", act, dui_action_get_name (act));

	/* Issue same query as before, and display the results */
	recs = dui_txnquery_rerun_last_query (act->query);
	dui_report_show_data (act->report, recs);
	dui_recordset_free (recs);
	
	LEAVE ("(act=%p \'%s\')", act, dui_action_get_name (act));
}

/* ========================== END OF FILE ============================ */
