/********************************************************************\
 * report.c --  Fill in widget values based on database query       *
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
 * Fill in values based on results of database query
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <qof/qof.h>

#include "perr.h"
#include "util.h"

#include "action.h"
#include "dui-initdb.h"
#include "duifield.h"
#include "duifield-gobj.h"
#include "duifield-gtk.h"
#include "duifield-hash.h"
#include "duifield-qof.h"
#include "duifield-sql.h"
#include "duifieldmap.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "duitxnreport.h"
#include "report.h"
#include "window.h"


struct DuiReport_s
{
	char * name;             /**< Our name */

	GList *rows;             /**< List of nested/tree row iterators */
	DuiTxnReport *curr_row;  /**< Default row, if no other specified. */

	/** list of actions that last drew into this window */
	GList *last_queries;

	/** Name resolution infrastructure. Please note similarity to the
	 * DuiAction infrastructure, this is not an accident. */
	DuiResolver  *resolver;
	DuiWindow    *window;     /**< window holding src/target widgets */
	DuiInterface *interface;  /**< global anchor */
	QofBook      *book;       /**< Book holding objects */
};

/* ============================================================ */

DuiReport * 
dui_report_new (const char * name)
{
	DuiReport * rpt;

	rpt = g_new0 (DuiReport, 1);

	rpt->name = g_strdup (name);
	rpt->rows = NULL;
	rpt->curr_row = NULL;
	rpt->last_queries = NULL;

	rpt->resolver = dui_resolver_new ();
	rpt->window = NULL;
	rpt->interface = NULL;
	rpt->book = NULL;
	return rpt;
}

void 
dui_report_destroy (DuiReport *rpt)
{
	GList *rnode;
	if (NULL == rpt) return;

	g_free(rpt->name);
	g_list_free (rpt->last_queries);

	for (rnode=rpt->rows; rnode; rnode=rnode->next)
	{
		DuiTxnReport *row = rnode->data;
		dui_txnreport_destroy (row);
	}
	g_list_free (rpt->rows);
	rpt->rows = NULL;

	dui_resolver_destroy (rpt->resolver);
	rpt->window = NULL;
	rpt->interface = NULL;
	rpt->book = NULL;
	g_free (rpt);
}

/* ============================================================ */

void
dui_report_set_last_action (DuiReport *rpt, DuiAction *act)
{
	if (!rpt) return;

	if (!act)
	{
		g_list_free (rpt->last_queries);
		rpt->last_queries = NULL;
	}

	if (!act) return;

	rpt->last_queries = g_list_append (rpt->last_queries,  act);
}

/* ============================================================ */

void
dui_report_add_row (DuiReport *rpt, DuiTxnReport *row)
{
	if (!rpt || !row ) return;
	
	rpt->rows = g_list_append (rpt->rows, row);
	dui_txnreport_set_resolver (row, rpt->resolver);
}

/* ============================================================ */

void
dui_report_add_term (DuiReport *rpt, DuiFieldMap *fm)
{
	if (!rpt || !fm) return;

	/* Its valid to have reports without a row iterator. However, 
	 * column definitions are stored with a row iterator, so ...
	 */
	if (NULL == rpt->curr_row)
	{
		DuiTxnReport *row;
		row = dui_txnreport_new (NULL, 0);
		dui_report_add_row (rpt, row);
		rpt->curr_row = row;
	}
	dui_txnreport_add_term (rpt->curr_row, fm);
}

/* ============================================================ */

int
dui_report_show_data (DuiReport *rpt, DuiDBRecordSet *recs)
{
	GList *node;

	if (!rpt) return 0;

	ENTER ("(rpt=%p \'%s\', recs=%p)", rpt, dui_report_get_name (rpt), recs);

	/* Check to see if there's any data to display at all.
	 * If there's not, then we don't realize the report widget, 
	 * we don't show anything, we bail from here, and let the
	 * next action in the chain take over. */
	int have_rows = 0;
	for (node=rpt->rows; node; node = node->next)
	{
		DuiTxnReport *row = node->data;

		if (!dui_txnreport_is_empty (row, recs)) 
		{
			have_rows = 1;
			break;
		}
	}
	if (0 == have_rows) return 0;

	/* Make widget show up on screen & other intialization */
	dui_window_realize (rpt->window);

	for (node=rpt->rows; node; node = node->next)
	{
		DuiTxnReport *row = node->data;

		if (dui_txnreport_is_empty (row, recs)) continue;
		
		/* dui_txnreport_realize (row); */
		dui_txnreport_run (row, recs);
	}

	LEAVE ("(rpt=%p \'%s\', recs=%p)", rpt, dui_report_get_name (rpt), recs);
	
	return 1;
}

/* ============================================================ */

const char * 
dui_report_get_name (DuiReport *rpt)
{
	if (!rpt) return NULL;
	if (rpt->name) return (rpt->name);
	return dui_window_get_name (rpt->window);  // XXX
}

/* ============================================================ */

void
dui_report_set_window (DuiReport *rpt, DuiWindow *dwin)
{
	if (!rpt) return;
	rpt->window = dwin;
}

void
dui_report_set_interface (DuiReport *rpt, DuiInterface *iface)
{
	if (!rpt) return;
	rpt->interface = iface;
}

void
dui_report_set_book (DuiReport *rpt, QofBook *book)
{
	if (!rpt) return;
	rpt->book = book;
}

void
dui_report_do_realize (DuiReport *rpt)
{
	if (!rpt) return;
	ENTER ("(rpt=%p \'%s\')", rpt, dui_report_get_name (rpt));

	/* Convert widget names into pointers to actual widgets */
	dui_resolver_resolve_widgets (rpt->resolver, rpt->window);

	/* Pull global stuff out of global anchor point */
	dui_resolver_resolve_gobj (rpt->resolver, rpt->interface);
	dui_resolver_resolve_hash (rpt->resolver, rpt->interface);
	dui_resolver_resolve_qof (rpt->resolver, rpt->book);

	LEAVE ("(rpt=%p \'%s\')", rpt, dui_report_get_name (rpt));
}

/* ============================================================ */

void
dui_report_refresh (DuiReport *rpt)
{
	GList *node;
	if (!rpt) return;

	ENTER ("(rpt=%p \'%s\')", rpt, dui_report_get_name (rpt));
	
	/* No refresh needed if the main window widget doesn't exist */
	/* XXX this is true only if report is not dumping data to 
	 * other places as well ?? XXX this is a bug, fixme ... */
	if (rpt->window && (0 == dui_window_is_realized (rpt->window))) return;

	for (node=rpt->last_queries; node; node=node->next)
	{
		DuiAction *last_action = node->data;
		dui_action_rerun_last_query (last_action);
	}
	LEAVE ("(rpt=%p \'%s\')", rpt, dui_report_get_name (rpt));
}

/* ============================== END OF FILE =================== */
