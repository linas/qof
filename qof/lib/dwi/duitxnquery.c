/********************************************************************\
 * duitxnquery.c -- Implementation of SQL Query Handler             *
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

#include "config.h"

#include <string.h>

#include <glib.h>

#include "builder.h"
#include "database.h"
#include "dui-initdb.h"
#include "duifield.h"
#include "duifieldmap.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "duitxnquery.h"
#include "perr.h"

typedef enum {
	QUERY_NONE,
	QUERY_SIMPLE,
	QUERY_TABLES,
	QUERY_FIELDS
} MetaQueryType;

/* Implements connection between the database and the fields
 * that acess the database */
struct DuiTxnQuery_s
{
	/** Special DB queries that don't use normal SQL channels */
	MetaQueryType meta_qtype;
	
	/** Table name(s) on which SQL queries will be done. 
	 *  Multiple table names can be specified with comma-separated 
	 *  list.  */
	char * tabname;

	/** Table for which we want to get the column names;
	 *  NULL if we are not interested in column names.  */
	DuiField *table;

	/** The query builder */
	SqlBuilder *sql_builder;

	/** A portion of the sql query */
	SqlBuilderQType qtype;

	GList *terms;

	/** Cache of most recent sql query string; used for rerunning the query. */
	char * query_string;

	/** Database connection on which to send the query */
	DuiDatabase *database;
	DuiDBConnection *db_conn;

	/* ------------------------------ */
	/** If the source is a multi-line table, 
	 * this helps identify the matching source row. */
	DuiFieldMap *source_match;

	/** Name resolution infrastructure */
	DuiResolver * resolver;
};

/* =================================================================== */

static void
dui_txnquery_init (DuiTxnQuery *q)
{
	q->meta_qtype = QUERY_NONE;
	q->tabname = NULL;
	q->table = NULL;
	q->sql_builder = NULL;

	q->qtype = 0;
	q->terms = NULL;

	q->query_string = NULL;
	q->database = NULL;
	q->db_conn = NULL;

	q->source_match = NULL;
	q->resolver = NULL;
}

DuiTxnQuery *
dui_txnquery_new (void)
{
	DuiTxnQuery *qry;
	qry = g_new0 (DuiTxnQuery, 1);
	dui_txnquery_init (qry);
	return qry;
}

void
dui_txnquery_destroy (DuiTxnQuery *q)
{
	GList *node;
	
	q->meta_qtype = QUERY_NONE;

	if (q->tabname) g_free (q->tabname);

	/* XXX should we dui_field_map_destroy() the table ?? */
	q->table = NULL;

	q->database = NULL;
	q->db_conn = NULL;

	/* Remove the insert/update terms */
	for (node=q->terms; node; node=node->next)
	{
		DuiFieldMap *fm = node->data;
		dui_field_map_destroy (fm);
	}
	g_list_free (q->terms);

	if (q->source_match) dui_field_map_destroy (q->source_match);

	sql_builder_destroy(q->sql_builder);
	q->sql_builder = NULL;
	q->qtype = 0;

	g_free (q);
}

/* =================================================================== */

void 
dui_txnquery_set_database (DuiTxnQuery * q, DuiDatabase *db)
{
	if (!q) return;
	q->database = db;
}
	
/* =================================================================== */

void 
dui_txnquery_set_tablename (DuiTxnQuery *qry, const char * tablename)
{
	ENTER ("(qry=%p, table=%s)", qry, tablename);
	if (!qry || !tablename) return;

	if (qry->tabname) g_free (qry->tabname);
	qry->tabname = g_strdup (tablename);

	/* If the query type has been set and its a basic sql query,
	 * then go ahead and crank up the builder. */
	if (QUERY_SIMPLE == qry->meta_qtype)
	{
		if (NULL == qry->sql_builder)
		{
			qry->sql_builder = sql_builder_new ();
		}
		sql_builder_table (qry->sql_builder, qry->tabname, qry->qtype);
	}
}

/* =================================================================== */

void 
dui_txnquery_set_querytype (DuiTxnQuery *qry, const char * sql_querytype)
{
	ENTER ("(qry=%p, type=%s)", qry, sql_querytype);
	if (!qry || !sql_querytype) return;

	if (!strcasecmp (sql_querytype, "tables"))
	{
		qry->meta_qtype = QUERY_TABLES;
		qry->table = NULL;
		if (qry->tabname) 
		{
			g_free (qry->tabname);
			qry->tabname = NULL;
		}
		return;
	}
	
	/* Set up the sql statement builder so that the next set of
	 * of add_term calls set up a valid sql statement. At this point,
	 * a table must be specified.
	 */
	qry->meta_qtype = QUERY_SIMPLE;
	if (!strcasecmp (sql_querytype, "select")) qry->qtype = SQL_SELECT;
	else if (!strcasecmp (sql_querytype, "insert")) qry->qtype = SQL_INSERT;
	else if (!strcasecmp (sql_querytype, "update")) qry->qtype = SQL_UPDATE;
	else if (!strcasecmp (sql_querytype, "delete")) qry->qtype = SQL_DELETE;
	else 
	{
		SYNTAX ("unknown query type \'%s\'", sql_querytype);
		return;
	}

	/* If we now have the table name, and the query is a basic sql query,
	 * then go ahead and crank up the builder. */
	if (qry->tabname)
	{
		if (NULL == qry->sql_builder)
		{
			qry->sql_builder = sql_builder_new ();
		}
		sql_builder_table (qry->sql_builder, qry->tabname, qry->qtype);
	}
}

/* =================================================================== */

void 
dui_txnquery_set_table (DuiTxnQuery *qry, DuiField * tabfld, 
                                      const char * sql_querytype)
{
	const char * tabname = NULL;

	if (!strcasecmp (sql_querytype, "fields"))
	{
		qry->meta_qtype = QUERY_FIELDS;
		qry->table = tabfld;
		return;
	}

	if (!tabfld->get_field_value) return;
	tabname = tabfld->get_field_value(tabfld);
	if (!tabname) return;

	dui_txnquery_set_tablename (qry, tabname);
	dui_txnquery_set_querytype (qry, sql_querytype);
}

/* =================================================================== */
/* Note: the following three routines (add_term, add_match_term,
 * set_resolver) are identical/almost identical to thier counterparts
 * in duitxnreport.c.   
 */

void 
dui_txnquery_add_term (DuiTxnQuery *qry, DuiFieldMap *fm)
{
	if (!qry || !fm) return;
	dui_resolver_add_field (qry->resolver, &fm->source);
	dui_resolver_add_field (qry->resolver, &fm->target);
	dui_field_map_resolve (fm);

	/* If its a SELECT term then we can handle it statically.
	 * The name of the SELECT field will be stored as a const target
	 */
	if (DUI_FIELD_IS_TYPE(&fm->target,DUI_FIELD_CONST))
	{
		sql_builder_set_str (qry->sql_builder, fm->target.u.value, NULL);
	}
	else
	{
		qry->terms = g_list_append (qry->terms, fm);
	}
}

void 
dui_txnquery_add_source_match_term (DuiTxnQuery *qry, DuiFieldMap *fm)
{
	if (!qry || !fm) return;

	if (qry->source_match)
	{
		PERR ("Source Row Matcher already specified!\n");
	}
	dui_resolver_add_field (qry->resolver, &fm->source);
	dui_resolver_add_field (qry->resolver, &fm->target);
	dui_field_map_resolve (fm);

	qry->source_match = fm;
}

DuiFieldMap *
dui_txnquery_get_source_match_term (DuiTxnQuery *qry)
{
	if (!qry) return NULL;
	return qry->source_match;
}

void 
dui_txnquery_set_resolver (DuiTxnQuery *qry, DuiResolver *res)
{
	GList *cnode;
	DuiFieldMap *fm;
	if (!qry) return;
	qry->resolver = res;

	/* Resolve any previously added fields */
	fm = qry->source_match;
	if (fm)
	{
		dui_resolver_add_field (res, &fm->source);
		dui_resolver_add_field (res, &fm->target);
		dui_field_map_resolve (fm);
	}

	for (cnode=qry->terms; cnode; cnode=cnode->next)
	{
		fm = cnode->data;
		dui_resolver_add_field (res, &fm->source);
		dui_resolver_add_field (res, &fm->target);
		dui_field_map_resolve (fm);
	}
}

/* =================================================================== */

void
dui_txnquery_do_realize (DuiTxnQuery *qry)
{
	/* Everything was resolved at the time the terms were added. 
	 * And that's good, there's no benefit, just complications
	 * and dependency-on-order-of-resolution problems with late resolution. 
	 */
}

/* =================================================================== */

void 
dui_txnquery_connect (DuiTxnQuery *qry)
{
	if (!qry) return;
	qry->db_conn = dui_database_do_realize (qry->database);
}

/* =================================================================== */

static DuiDBRecordSet *
dui_txnquery_run_one_query (DuiTxnQuery *qry)
{
	DuiDBRecordSet *recs = NULL;
	if (!qry) return NULL;

	ENTER ("(qry=%p)", qry);
	switch (qry->meta_qtype)
	{
		case QUERY_NONE: break;
		case QUERY_SIMPLE: break;
		case QUERY_TABLES:
			if (qry->db_conn)
			{
				recs = dui_connection_tables(qry->db_conn);
				return recs;
			}
			break;
		case QUERY_FIELDS:
			if ((qry->db_conn) && (qry->table->get_field_value))
			{
				const char * table;
				table = qry->table->get_field_value(qry->table);
				PINFO ("perform fields query with table=\'%s\'", table);
				recs = dui_connection_table_columns(qry->db_conn, table);
				return recs;
			}
			break;
	}

	/* If we have data we need to query, then do the query.  */
	if (qry->sql_builder)
	{
		GList *node;
		SqlBuilder *dupe;
		short broken_select = 0;
		short wild_select = 1;

		g_free (qry->query_string);
		qry->query_string = NULL;

		/* Make a copy of the query, which might be holding statically
		 * compiled 'SELECT term FROM table' in it.  These static terms
		 * were added with the dui_txnquery_add_term() routine above.
		 * We append dynamic terms below, dynamic in that we need the 
		 * values for the INSERT, UPDATE and WHERE terms.
		 *
		 * Note also: if the sql_builder is empty, then there MUST be 
		 * some insert/update terms; alternately, if the sql_builder 
		 * is not empty, there must NOT be any insert/update terms,
		 * else bad SQL will be generated. 
		 */
		dupe = sql_builder_copy (qry->sql_builder);
	
		/* Loop, looking for INSERT/UPDATE terms */
		for (node = qry->terms; node; node=node->next)
		{
			DuiFieldMap *fm = node->data;
			const char * fieldval;
			if ((!DUI_FIELD_IS_TYPE(&fm->target,DUI_FIELD_SQL)) &&
			    (!DUI_FIELD_IS_TYPE(&fm->target,DUI_FIELD_WHERE))) continue;
			
			fieldval = dui_field_map_get_value (fm);
			if ((NULL == fieldval) && ((SQL_INSERT == qry->qtype) ||
			                           (SQL_UPDATE == qry->qtype)))
			{
				PERR ("Null field value for insert/update not allowed, field=%s\n", 
				       fm->target.fieldname);
				sql_builder_destroy (dupe);
				return NULL;
			}
			sql_builder_set_str (dupe, fm->target.fieldname, fieldval);
			PINFO ("insert/update %s=%s", fm->target.fieldname, fieldval);
		}
	
		/* Loop again, this time for where terms */
		for (node = qry->terms; node; node=node->next)
		{
			DuiFieldMap *fm = node->data;
			const char * fieldval;
			
			if (!DUI_FIELD_IS_TYPE(&fm->target,DUI_FIELD_WHERE)) continue;
			wild_select = 0;
	
			fieldval = dui_field_map_get_value (fm);
			PINFO ("where (%s %s %s)", fm->target.fieldname, 
			                dui_field_where_get_op(&fm->target), fieldval);
			/* fields left blank don't match anything */
			fieldval = whitespace_filter (fieldval);
			if (fieldval)
			{
				/* Do not use 'where' term when inserting;
				 * This was already picked up above. */
				if (SQL_INSERT != qry->qtype)
				{
					sql_builder_where_str (dupe,
					   fm->target.fieldname, 
						fieldval, dui_field_where_get_op(&fm->target));
				}
			}
			else
			{
				broken_select = 1;
			}
		}
	
		if (NULL == qry->db_conn)
		{
			/* Now get the database too */
			if (NULL == qry->database)
			{
				PERR ("Can't perform query, no database specified!\n");
				sql_builder_destroy (dupe);
				return NULL;
			}
			qry->db_conn = dui_database_do_realize (qry->database);
		}

		/* Perform the query only if its an intentional wild-card
		 * or has valid WHERE terms.  Goal is to try to prevent
		 * stupid DUI XML file errors from doing a global clobber.
		 * (e.g. an unintentional wildcard delete, because some
		 * value for the where clause couldn't be found.)
		 */
		if (wild_select || !broken_select)
		{
			const char * stmt;

			/* Get the sql statement string */
			stmt = sql_builder_query (dupe);
			qry->query_string = g_strdup (stmt);
	
			/* Now run the query */
			if (qry->db_conn)
			{
				recs = dui_connection_exec(qry->db_conn, stmt);
			}
		}
		else
		{
			PERR ("Can't perform query, missing WHERE term (%d %d)",
			       wild_select, broken_select);
			sql_builder_destroy (dupe);
			return NULL;
		}
		sql_builder_destroy (dupe);
	}
	else
	{
		PERR ("Failed to specify table for query");
	}

	LEAVE(" got recs=%p for query=\"%s\"\n", recs, qry->query_string);

	return recs;
}

DuiDBRecordSet *
dui_txnquery_run (DuiTxnQuery *qry)
{
	DuiDBRecordSet *rec_return = NULL;

	int more_rows = 1;
	DuiField *iterator = NULL;
	if (!qry) return NULL;

	ENTER ("(qry=%p)", qry);
	/* Get set to handle source row */
	if (qry->source_match)
	{
		iterator = &qry->source_match->target;
	}

	dui_field_iter_pre (iterator, TRUE);

	/* XXX Ideally, this should be a loop to handle multiple
	 * rows, but we can't do that because we have no way of
	 * returning multiple query results.  We'd have to 
	 * abstract the concept of the recordset ... and yes, 
	 * we should do this eventually. 
	 * 
	 * Note that this loop is/should be identical to the 
	 * dui_txnreport_run() loop over rows.  The existing difference
	 * is due to teh fact that the recordset concept hasn't 
	 * been abstracted away from te SQL, and so we cannot use
	 * it here.  The recordset should be abstracted.
	 */
	more_rows = 1;
	while (more_rows)
	{
		if (qry->source_match)
		{
			dui_field_map_transfer_data (qry->source_match);
		}
	
		gboolean do_transfer = dui_field_iter_next (iterator);
	
		if (do_transfer)
		{
			GList *node;
			for (node=qry->terms; node; node=node->next)
			{
				DuiFieldMap *fm = node->data;
				dui_field_iter_column (&fm->source, iterator);
			}

			rec_return = dui_txnquery_run_one_query (qry);
		}

		more_rows = 0;
	}
	
	dui_field_iter_post (iterator);
	LEAVE ("(qry=%p)", qry);
	return rec_return;
}

/* =================================================================== */

DuiDBRecordSet *
dui_txnquery_rerun_last_query (DuiTxnQuery *qry)
{
	DuiDBRecordSet *recs = NULL;

	if (!qry) return NULL;
	if (!qry->db_conn || !qry->query_string) return NULL;

	recs = dui_connection_exec (qry->db_conn, qry->query_string);
	return recs;
}

/* ========================== END OF FILE ============================ */
