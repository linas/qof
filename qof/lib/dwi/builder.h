/********************************************************************\
 * builder.h : compile SQL queries from C language data             *
 * Copyright (C) 2001 Linas Vepstas <linas@linas.org>               *
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
 * Generic SQL query builder.  This class can be sued to construct
 * a basic sql query statement (of the type 'select', 'update' or 
 * 'insert') by simply making C calls indicating the table and the 
 * fields to query.
 *
 * Its fairly limited in the range of sql syntax that it supports, 
 * but on the other hand, the code here is/should be general enough 
 * to work with  any SQL implementation.
 * 
 * HISTORY:
 * Linas Vepstas January 2001
 */

#ifndef SQL_BUILDER_H
#define SQL_BUILDER_H

#include <time.h>

#define SQL_DBL_FMT "%24.18g"

typedef enum {
   SQL_UPDATE = 'm',  /* m == modify */
   SQL_INSERT = 'a',  /* a == add */
   SQL_SELECT = 'q',  /* q == query */
   SQL_DELETE = 'd'   /* d == drop, delete */
} SqlBuilderQType;

typedef struct SqlBuilder_s SqlBuilder; 

/* The sql_builder_copy() routine makes a copy of the indicated object.
 *    Note that this routine is particularly useful for making a copy of
 *    a half-finished statement, and then using this copy to e.g. try out 
 *    different 'where' clauses.
 */

SqlBuilder * sql_builder_new(void);
SqlBuilder * sql_builder_copy (SqlBuilder *);
void sql_builder_destroy (SqlBuilder *);

/* The sql_builder_table() routine starts building a new SQL query 
 *    on table 'tablename'.  Any previously started query is erased.
 *
 *    When building 'select' type statments, crude table joins are 
 *    supported: the 'tablename' can in fact be a comma-separated list
 *    of tables.  This field is copied directly as follows:
 *    "SELECT ... FROM tablename WHERE ..." so anything valid in that
 *    position is tolerated.
 */
void sql_builder_table (SqlBuilder *b,
                     const char *tablename,
                     SqlBuilderQType qtype);


/* Set tag-value pairs.  Each of these adds the indicated
 * tag and value to an UPDATE or INSERT statement.  For SELECT
 * statements, val may be NULL (and is ignored in any case).
 */
void sql_builder_set_str   (SqlBuilder *b, const char *tag, const char *val);
void sql_builder_set_char  (SqlBuilder *b, const char *tag, char val);
void sql_builder_set_date  (SqlBuilder *b, const char *tag, time_t val);
void sql_builder_set_int64 (SqlBuilder *b, const char *tag, gint64 val);
void sql_builder_set_int32 (SqlBuilder *b, const char *tag, gint32 val);
void sql_builder_set_double(SqlBuilder *b, const char *tag, double val);


/* The sql_builder_where_*() routines are used to construct the 'WHERE'
 *    part of SQL SELECT and UPDATE clauses.  The 'tag' must be a valid
 *    fieldname, and 'val' must be a value.  If 'op' is NULL, it is assumed
 *    to be '=', otherwise, one can specify an op '<', '>' and so on.
 */

void sql_builder_where_str  (SqlBuilder *b, const char *tag, 
                                     const char *val, const char *op);
void sql_builder_where_int32 (SqlBuilder *b, const char *tag, 
                                       gint32 val, const char *op);


/* The sql_builder_query() routine returns a valid SQL query 
 *    statement that reflects the set of build calls just made.   
 *    This string is clobbered when sql_builder_destroy() or 
 *    sql_builder_Table() is called, so make a copy if you need it.
 *
 *    This resulting query string is probably general enough to 
 *    work with almost any SQL db, I beleive. 
 */
const char *sql_builder_query (SqlBuilder *b);



#endif /* SQL_BUILDER_H */

