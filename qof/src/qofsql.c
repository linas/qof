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

#include <glib.h>
#include <libsql/sql_parser.h>
#include "qofbook.h"
#include "qofquery.h"
#include "qofsql.h"

struct _QofSqlQuery
{
	sql_statement *parse_result;
	QofQuery *qof_query;
	char * single_global_tablename;
};

/* ========================================================== */

QofSqlQuery *
qof_sql_query_new(void)
{
	QofSqlQuery * sqn = (QofSqlQuery *) g_new (QofSqlQuery, 1);
	
	sqn->qof_query = NULL;
	sqn->parse_result = NULL;
	sqn->single_global_tablename = NULL;

	return sqn;
}

/* ========================================================== */

void 
qof_sql_query_destroy (QofSqlQuery *q)
{
	if (!q) return;
	qof_query_destroy (q->qof_query);
	sql_destroy (q->parse_result);
	g_free (q);
}

/* ========================================================== */

void 
qof_sql_query_set_book (QofSqlQuery *q, QofBook *book)
{
	if (!q) return;
	qof_query_set_book (q->qof_query, book);
}

/* ========================================================== */

static inline void
get_table_and_param (char * str, char **tab, char **param)
{
	char * end = strchr (str, '.');
	if (!end) 
	{
		*tab = 0;
		*param = str;
		return;
	}
	*end = 0;
	*tab = str;
	*param = end+1;
}

static QofQuery *
handle_single_condition (sql_condition * cond, char *globalname)
{
	GSList *param_list;
	QofQueryPredData *pred_data = NULL;
	
	if (NULL == cond)
	{
		printf ("Error: missing condition\n");
		return NULL;
	}
			
	/* -------------------------------- */
	/* field to match, assumed, for now to be on the left */
	/* XXX fix the left-right thing */
	if (NULL == cond->d.pair.left)
	{
		printf ("Error: missing left paramter\n");
		return NULL;
	}
	sql_field_item * sparam = cond->d.pair.left->item;
	if (SQL_name != sparam->type)
	{
		printf ("Error: we support only paramter names\n");
		return NULL;
	}
	char * qparam_name = sparam->d.name->data;
	if (NULL == qparam_name)
	{
		printf ("Error: we missing paramter name\n");
		return NULL;
	}

	/* -------------------------------- */
	/* value to match, assumed, for now, to be on the right. */
	/* XXX fix the left-right thing */
	if (NULL == cond->d.pair.right)
	{
		printf ("duude missing right paramter\n");
		return NULL;
	}
	sql_field_item * svalue = cond->d.pair.right->item;
	if (SQL_name != svalue->type)
	{
		printf ("Error: we support only simple values\n");
		return NULL;
	}
	char * qvalue_name = svalue->d.name->data;
	if (NULL == qvalue_name)
	{
		printf ("Error: we missing value\n");
		return NULL;
	}

	/* -------------------------------- */
	/* Now start building the QOF paramter */
	param_list = qof_query_build_param_list (qparam_name, NULL);

	/* Get the where-term comparison operator */
	QofQueryCompare qop;
	switch (cond->op)
	{
		case SQL_eq: qop = QOF_COMPARE_EQUAL; break;
		case SQL_gt: qop = QOF_COMPARE_GT; break;
		case SQL_lt: qop = QOF_COMPARE_LT; break;
		case SQL_geq: qop = QOF_COMPARE_GTE; break;
		case SQL_leq:   qop = QOF_COMPARE_LTE; break;
		case SQL_diff:  qop = QOF_COMPARE_NEQ; break;
		default:
			printf ("Error: unsupported compare op for now\n");
			return NULL;
	}

	/* OK, need to know the type of the thing being matched 
	 * in order to build the correct predicate.  Get the type 
	 * from the object parameters. */
	char *table_name;
	char *param_name;
	get_table_and_param (qparam_name, &table_name, &param_name);
	if (NULL == table_name)
	{
		table_name = globalname;
	}
		
	if (NULL == table_name)
	{
		printf ("Error: Need to specify a table to query\n");
		return NULL;
	}
			
	QofType param_type = qof_class_get_parameter_type (table_name, param_name);
printf ("duude parse table=%s param=%s type=%s\n",
table_name, param_name, param_type);

	if (!strcmp (param_type, QOF_TYPE_STRING))
	{
		/* strip out quotation marks ...  */
		if (('\'' == qvalue_name[0]) ||
		    ('\"' == qvalue_name[0]))
		{
			qvalue_name ++;
			size_t len = strlen(qvalue_name);
			qvalue_name[len-1] = 0;
		}
		pred_data = 
		    qof_query_string_predicate (qop, /* comparison to make */
		    qvalue_name,                     /* string to match */
            QOF_STRING_MATCH_CASEINSENSITIVE,  /* case matching */
       	    FALSE);                            /* use_regexp */
	}
	else
	{
		printf ("Error: predicate type unsupported for now \n");
		return NULL;
	}

	QofQuery *qq = qof_query_create();
	qof_query_add_term (qq, param_list, pred_data, QOF_QUERY_FIRST_TERM);
	return qq;
}

/* ========================================================== */

static QofQuery *
handle_where (QofSqlQuery *query, sql_where *swear)
{
	switch (swear->type)
	{
		case SQL_pair:
		{
			printf ("duuude unhandled sql pair\n");
			// QofQuery * qof_query_merge(QofQuery *q1, QofQuery *q2, QofQueryOp op)
			return NULL;
			break;
		}
		case SQL_negated:
			// qof_query_invert ... 
			printf ("duuude unhandled sql negated\n");
			return NULL;
			break;
		case SQL_single:
		{
			GSList *param_list;
			QofQueryPredData *pred_data = NULL;
			
			sql_condition * cond = swear->d.single;
			return handle_single_condition (cond, query->single_global_tablename);
		}
	}
	return NULL;
}

/* ========================================================== */

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

	if (SQL_select != query->parse_result->type)
	{
		printf ("Error: currently, only SELECT statements are supported, "
		                     "got type=%d\n", query->parse_result);
		return NULL;
	}

	/* If the user wrote "SELECT * FROM tablename WHERE ..."
	 * then we have a single global tablename.  But if the 
	 * user wrote "SELECT * FROM tableA, tableB WHERE ..."
	 * then we don't have a single unique table-name.
	 */
	GList *tables = sql_statement_get_tables (query->parse_result);
	if (1 == g_list_length (tables))
	{
		query->single_global_tablename = tables->data;
	}

	sql_select_statement *sss = query->parse_result->statement;
	sql_where * swear = sss->where;
	if (NULL == swear)
	{
		printf ("Error: expecting 'where' statement\n");
		return NULL;
	}

	/* Walk over the where terms, turn them into QOF predicates */
	query->qof_query = handle_where (query, swear);
	if (NULL == query->qof_query) return NULL;

	/* We also want to set the type of thing to search for.
	 * If the user said SELECT * FROM ... then we should return
	 * a list of QofEntity.  Otherwise, we return ... ?
	 * XXX all this needs fixing.
	 */
	qof_query_search_for (query->qof_query, query->single_global_tablename);

qof_query_print (query->qof_query);
	GList *results = qof_query_run (query->qof_query);

	return results;

#if 0
	GList *fields = sql_statement_get_fields (query->parse_result);
	for (node=fields; node; node=node->next)
	{
		char *fieldname = node->data;
	}


query: select * from myobj where myobj.thing=1;
		   fields:
		     *
			from:
			   table: myobj
			   where:
			     op: =
			    left:
			myobj.thing
			   right:
		        1
		                      
		       Tables: myobj
		      Fields: *
		 
#endif
}

/* ========================== END OF FILE =================== */
