/********************************************************************\
 * qofsql.h -- QOF cleint-side SQL parser                           *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/**
    @file qofsql.c
    @breif QOF client-side SQL parser.
    @author Copyright (C) 2004 Linas Vepstas <linas@linas.org>
*/

#ifndef QOF_SQL_QUERY_H
#define QOF_SQL_QUERY_H

#include <glib.h>
#include "qofbook.h"

typedef struct _QofSqlQuery QofSqlQuery;

/** Create a new SQL-syntax query machine.
 */
QofSqlQuery * qof_sql_query_new (void);
void qof_sql_query_destroy (QofSqlQuery *);

/** Set the book to be searched (you can search multiple books)
 *  If no books are set, no results will be returned (since there
 *  is nothing to search over).
 */
void qof_sql_query_set_book (QofSqlQuery *q, QofBook *book);

/** Perform the query, return the results.
 *
 *  The returned list is a list of the 'search-for' type that was
 *  previously set with the qof_query_search_for() or the
 *  XXX fixme this doc is wrong.
 *  qof_query_create_for() routines.  The returned list will have
 *  been sorted using the indicated sort order, and trimed to the
 *  max_results length.
 *  Do NOT free the resulting list.  This list is managed internally
 *  by QofSqlQuery.
 */

GList * qof_sql_query_run (QofSqlQuery *query, const char * str);

#endif /* QOF_SQL_QUERY_H */

#include <libqofsql/sql_parser.h>
#include "qofquery.h"

struct _QofSqlQuery
{
	QofQuery *qof_query;
	sql_statement *parse_result;
};

QofSqlQuery *
qof_sql_query_new(void)
{
	QofSqlQuery * sqn = (QofSqlQuery *) g_new (QofSqlQuery);
	
	sqn->qof_query = qof_query_create ();
	sqn->parse_result = NULL;

	return sqn;
}

void 
qof_sql_query_destroy (QofSqlQuery *q)
{
	if (!q) return;
	qof_query_destroy (q->qof_query);
	sql_destroy (sqn->parse_result);
	g_free (q);
}

void 
qof_sql_query_set_book (QofSqlQuery *q, QofBook *book)
{
	if (!q) return;
	qof_query_set_book (q->qof_query, book);
}

GList * 
qof_sql_query_run (QofSqlQuery *query, const char *str)
{
	GList *node;

	if (!query) return NULL;
	query->parse_result = sql_parse (str);

	if (!query->parse_result) 
	{
		printf ("parse error\n"); // XXX replace 
		return NULL;
	}

	if (SQL_select != query->parse_result)
	{
		printf ("only support the select type\n");
		return NULL;
	}

	sql_select_statement *sss = query->parse_result->statement;
	sql_where swear = sss->where;
	if (NULL == swear)
	{
		printf ("expecting 'where' statement\n");
		return NULL;
	}

	switch (swear)
	{
		case SQL_pair:
			printf ("duuude unhandled\n");
			break;
		case SQL_negated:
			printf ("duuude unhandled\n");
			break;
		case SQL_single:
		{
			GSList *param_list;
			QofQueryPredData *pred_data;
			
			/* field to match, assumed, for now to be on the left */
			/* XXX fix the left-right thing */
			param_list = qof_query_build_param_list (where->d.pair.left, NULL);

			sql_condition * cond = where->d.single;
			if (NULL == cond)
			{
				printf ("missing condition\n");
				return NULL;
			}
			QofQueryCompare qop;
			switch (cond->op)
			{
				case SQL_eq: qop = QOF_COMPARE_EQUAL; break;
				case SQL_gt: qop = QOF_COMPARE_GT; break;
				case SQL_lt: qop = QOF_COMPARE_LT; break;
				case SQL_geq: qop = QOF_COMPARE_GTE; break;
				case SQL_leq:   qop = QOF_COMPARE_LTE; break;
				case SQL_diff:  qop = QOF_COMPARE_NEQ; break;
			}
			
			pred_data = qof_query_string_predicate (qop, /* comparison to make */

		 "M M M My Sharona",                /* string to match */
	          QOF_STRING_MATCH_CASEINSENSITIVE,  /* case matching */
	         FALSE);                            /* use_regexp */

			printf ("duuude unhandled\n");
			break;
		}
	}

	GList *tables = sql_statement_get_tables (query->parse_result);
	GList *fields = sql_statement_get_fields (query->parse_result);

	for (node=fields; node; node=node->next)
	{
		char *fieldname = node->data;
	}

query: select * from myobj where myobj.thing=1;
		   fields:
		     *
						 *   from:
						 *       table: myobj
						 *         where:
						 *               op: =
						 *                     left:
						 *                             myobj.thing
						 *                                   right:
						 *                                           1
						 *                                            
						 *                                            Tables: myobj
						 *                                            Fields: *
						 *
}

int main ()
{
	char * str = "SELECT * from MyObject";

	QofSqlQuery * qsq = qof_sql_query_new();
			  
	qof_sql_query_set_book (qsq,...);

	qof_sql_query_run (qsq, str);


}

