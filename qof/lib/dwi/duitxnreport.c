/********************************************************************\
 * duitxnreport.c -- copy from recordset to field targets.          *
 * Copyright (C) 2002, 2003 Linas Vepstas <linas@linas.org>         *
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
 * A 'txnreport' will fill in a bunch of field values, based on 
 * a previously obtained collection of SQL records (i.e. the results
 * of a previous SQL query.) The field values typically filled in
 * will be a bunch of gtk widgets, and/or possibly a gtk table widget.
 * The txnreport can handle multiple table rows in the latter case.
 *
 * HISTORY:
 * Copyright (c) 2002, 2003 Linas Vepstas <linas@linas.org>
 */

#include "config.h"

#include <glib.h>
#include <string.h>

#include "perr.h"

#include "duifield.h"
#include "duifield-gtk.h"
#include "duifield-sql.h"
#include "duifieldmap.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "duitxnreport.h"

struct DuiTxnReport_s
{
	char * row_name;    /**  hmm .. fieldmap name */

	GList *columns;     /**< list of columns */

	int nest_level;     /**< clear the table if 0 */

	/** Matching fieldmap.  The source of the fieldmap
	 *  contains the data that must be matched.  The target of 
	 *  the fieldmap specifies the table row that will be 
	 *  written to.  XXX this should really be a list, or,
	 * better yet, a generalized query. */
	DuiFieldMap *match;

	/** Name resolution infrastructure */
	DuiResolver * resolver;
};

/* ============================================================ */

DuiTxnReport *
dui_txnreport_new (const char * name, int nest)
{
	DuiTxnReport *row;
	
	row = g_new (DuiTxnReport, 1);
	row->row_name = NULL;
	if (name) row->row_name = g_strdup (name);

	row->columns = NULL;
	row->nest_level = nest;
	row->match = NULL;

	return row;
}

/* ============================================================ */

void 
dui_txnreport_destroy (DuiTxnReport *row)
{
	GList *cnode;

	g_free (row->row_name);
	dui_field_map_destroy (row->match);

	for (cnode=row->columns; cnode; cnode=cnode->next)
	{
		DuiFieldMap *fm = cnode->data;
		dui_field_map_destroy (fm);
	}
	g_list_free (row->columns);

	g_free (row);
}

/* ============================================================ */

const char * 
dui_txnreport_get_name (DuiTxnReport *row)
{
	if (!row) return NULL;
	return row->row_name;
}

int
dui_txnreport_get_nest (DuiTxnReport *row)
{
	if (!row) return 0;
	return row->nest_level;
}

/* ============================================================ */

void
dui_txnreport_add_term (DuiTxnReport *row, DuiFieldMap *fm)
{
	if (!row || !fm) return;
	dui_resolver_add_field (row->resolver, &fm->source);
	dui_resolver_add_field (row->resolver, &fm->target);
	dui_field_map_resolve (fm);
	
	row->columns = g_list_append (row->columns, fm);
}

void
dui_txnreport_add_match_term (DuiTxnReport *row, DuiFieldMap *fm)
{
	if (!row || !fm) return;

	if (row->match)
	{
		PERR ("Target Row Matcher already specified!\n");
	}
	dui_resolver_add_field (row->resolver, &fm->source);
	dui_resolver_add_field (row->resolver, &fm->target);
	dui_field_map_resolve (fm);
	
	row->match = fm;
}

void
dui_txnreport_set_resolver (DuiTxnReport * row, DuiResolver *res)
{
	GList *cnode;
	DuiFieldMap *fm;
	if (!row) return;
	row->resolver = res;

	/* Resolve any previously added fields */
	fm = row->match;
	if (fm)
	{
		dui_resolver_add_field (res, &fm->source);
		dui_resolver_add_field (res, &fm->target);
		dui_field_map_resolve (fm);
	}

	for (cnode=row->columns; cnode; cnode=cnode->next)
	{
		fm = cnode->data;
		dui_resolver_add_field (res, &fm->source);
		dui_resolver_add_field (res, &fm->target);
		dui_field_map_resolve (fm);
	}
}

/* ============================================================ */
/* XXX  we should also run the matching conditions here,
 * since its empty if no match.
 */

gboolean 
dui_txnreport_is_empty (DuiTxnReport * row, DuiDBRecordSet *recs)
{
	int more_rows = 1;

	if (!row) return TRUE;

	/* Check to see if there's any data to display at all.  Note
	 * that 'recs' may be NULL; there could still be static data
	 * or non-sql data. So must go through loop at least once.  
	 * Note also that the empty string "" is a valid value, and 
	 * its used to clear out/empty target fields.
	 *
	 * However, if DB returned zero rows, then the DB may be offline, 
	 * or the query may have been badly constructed.  Should check
	 * for these errors ... 
	 */
	dui_recordset_rewind (recs);
	while (more_rows)
	{
		GList *cnode;
		for (cnode=row->columns; cnode; cnode=cnode->next)
		{
			DuiFieldMap *col = cnode->data;
			const char * val;

			dui_field_resolve_recordset (&col->source, recs);
			val = dui_field_map_get_value (col);
			if (val) 
			{
				PINFO ("have data rows for row=%p %s", row, row->row_name);
				return FALSE; 
			}
		}
				
		more_rows = dui_recordset_fetch_row (recs);
	}
	PINFO ("empty row=%p %s", row, row->row_name);
	return TRUE;
}
	
/* ============================================================ */
/* Run the rows */

void 
dui_txnreport_run (DuiTxnReport * row, DuiDBRecordSet *recs)
{
	int more_rows = 1;
	DuiField *iterator = NULL;

	if (!row) return;
	ENTER ("(rowname=%s, recs=%p)", row->row_name, recs);

	/* Handle the matching term first */
	if (row->match) 
	{
		iterator = &row->match->target;
	}

	dui_field_iter_pre (iterator, (0 == row->nest_level));

	/* Note, we must pass through this at least once, even
	 * if there  are no rows in the recordset.  That's because
	 * some fields will be static, not coming from SQL
	 * 
	 * However, if DB returned zero rows, then the DB may be offline, 
	 * or the query may have been badly constructed.  Should check
	 * for these errors...
	 */
	dui_recordset_rewind (recs);
	more_rows = 1;
	while (more_rows)
	{
		/* Fetch the match value, if its been set */
		if (row->match)
		{ 
			dui_field_resolve_recordset (&row->match->source, recs);
			dui_field_map_transfer_data (row->match);
		}
		
		gboolean do_transfer = dui_field_iter_next (iterator);

		if (do_transfer)
		{
			GList *cnode;
			for (cnode=row->columns; cnode; cnode=cnode->next)
			{
				DuiFieldMap *col = cnode->data;
	
				dui_field_resolve_recordset (&col->source, recs);
				dui_field_iter_column (&col->target, iterator);
	
				/* OK, finally move data from source to dest */
				dui_field_map_transfer_data (col);
			}
		}

		more_rows = 0;
		if (recs) more_rows = dui_recordset_fetch_row (recs);
		PINFO ("recs=%p more_rows=%d", recs, more_rows);
	}

	dui_field_iter_post (iterator);
	LEAVE ("(rowname=%s) iterator=%p", row->row_name, iterator);
}

/* ============================== END OF FILE =================== */
