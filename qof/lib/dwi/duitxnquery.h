/********************************************************************\
 * duitxnquery.h -- Builder for SQL Queries                         *
 * Copyright (C) 2002, 2003,2004 Linas Vepstas <linas@linas.org>    *
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
/**
 * @file: duitxnquery.h
 * @author: Copyright (c) 2002,2003,2004 Linas Vepstas <linas@linas.org>
 *
 * @addtogroup TxnQuery
 * Handler for database queries.  DuiFields are used to construct an
 * SQL query, and then run that query.  Running the query returns 
 * a set of records.  The resulting records can be pumpted into 
 * a different set of fields by using the DuiTxnReport structures.
 *
 * @{
 */

#ifndef DUI_TXN_QUERY_H_
#define DUI_TXN_QUERY_H_

#include "database.h"
#include "dui-initdb.h"
#include "duifieldmap.h"
#include "duiresolver.h"

typedef struct DuiTxnQuery_s DuiTxnQuery;

DuiTxnQuery * dui_txnquery_new (void);
void dui_txnquery_destroy (DuiTxnQuery *q);

/** Specify the sql table that will be queried.
 *  sql_querytype must be one of "select", "update", "insert", "delete",
 *        "tables" or "fields".
 *  If the querytype is "tables", then the 'tabfld' may be NULL;
 *  in all other cases, the 'tabfld', when evaluated, must return 
 *  the name of a valid SQL table.
 */
/* XXX qeuerytype should probably be another fieldmap term. 
 * holding query type in source and table name in target.
 */
void dui_txnquery_set_table (DuiTxnQuery *qry, DuiField *tabfld, 
                             const char * sql_querytype);

/** Specify the table name, or names, to be queried. If multiple
 *  table names are specified, they must be comma-separated (as
 *  they would normally be in an SQL statement).
 */
void dui_txnquery_set_tablename (DuiTxnQuery *qry, 
                                 const char * tablename);

/** Specify the sql query to be performed.
 *  sql_querytype must be one of "select", "update", "insert", "delete",
 *        "tables" or "fields".
 *  The first four correspiond to the standard SQL query types.
 *  The "fields" query type can be used to obtain the column names
 *  of a given table.  Handy if you don't yet know the column names.
 *  The "tables" query type can be used to obtain a listing of all
 *  the table names in the database.
 */
void dui_txnquery_set_querytype (DuiTxnQuery *qry, 
                                 const char * sql_querytype);

/** Add a term to the SQL query.  
 * For SELECT field names, set the fieldmap target to DUI_FIELD_CONST.
 * For UPDATE field & new value, set the fieldmap target to DUI_FIELD_SQL.
 * For WHERE macthing terms, set the fieldmap target to DUI_FIELD_WHERE.
 *
 * The memory management for the fieldmap passed in is taken over by
 * this routine, and when the query is deleted, the fieldmap will be
 * too.
 */
void dui_txnquery_add_term (DuiTxnQuery *qry, DuiFieldMap *fm);

/** For when the value sources are coming from a table, this specifies 
 *  the way in which the source table row is matched.
 *
 * The memory management for the fieldmap passed in is taken over by
 * this routine, and when the query is deleted, the fieldmap will be
 * too.
 */
void dui_txnquery_add_source_match_term (DuiTxnQuery *qry, DuiFieldMap *fm);
DuiFieldMap * dui_txnquery_get_source_match_term (DuiTxnQuery *qry);

void dui_txnquery_set_resolver (DuiTxnQuery *qry, DuiResolver *);
void dui_txnquery_set_database (DuiTxnQuery * q, DuiDatabase *db);

/** the txnquery_do_realize is currently a no-op.  and should stay that way.  
 *  XXX remove this routine ... right? ... */
void dui_txnquery_do_realize (DuiTxnQuery *qry);

/** Establish connection to the SQL db. XXX this should probably 
 * be done automatically, at query-run time.  We don't/shouldn't need
 * a separate step here, right? */
void dui_txnquery_connect (DuiTxnQuery *qry);

DuiDBRecordSet * dui_txnquery_run (DuiTxnQuery *qry);
DuiDBRecordSet * dui_txnquery_rerun_last_query (DuiTxnQuery *qry);

#endif /* DUI_DB_QUERY_H_ */

/** @} */
