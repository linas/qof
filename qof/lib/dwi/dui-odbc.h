/********************************************************************\
 * dui-odbc.h -- DUI ODBC driver                                    *
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
 * Open up connection to the database. (Uses ODBC under the covers).
 *
 * HISTORY:
 * Copyright (c) 2002 Linas Vepstas <linas@linas.org>
 * Created by Linas Vepstas  March 2002
 */


#ifndef DUI_ODBC_H
#define DUI_ODBC_H

#if USE_ODBC

#include "dui-initdb.h"

/* Call before using anything else */
void dui_odbc_init(void);

/** establish new connection to indicated database */
DuiDBConnection * dui_odbc_connection_new (const char * dbname, 
                                           const char * username,
                                           const char * authentication_token);

void dui_odbc_connection_free (DuiDBConnection *conn);

/* Issue a query on this connection.  FYI -- the query is actually performed
 * asynchronously; the record set may not yet contain valid data when this 
 * routine returns.  */
DuiDBRecordSet * dui_odbc_connection_exec (DuiDBConnection *, 
                const char * buff);

/* get a list of tables in this database */
DuiDBRecordSet * dui_odbc_connection_tables (DuiDBConnection *);
 
/* get a list of columns in the named table */
DuiDBRecordSet * dui_odbc_connection_table_columns (DuiDBConnection *,
                      const char * table_name);

/* release the record set when one is done with it */
void dui_odbc_recordset_release (DuiDBRecordSet *);

/* Prep a row.  Basically, this gets the 'next' row out of the database.
 * Subsequent field fetches are for this row. Returns zero if there are 
 * no more rows.
 */
int dui_odbc_recordset_fetch_row (DuiDBRecordSet *rs);

/* Get the field value by name.  This is a little different/easier than 
 * the usual trick of getting it by column number.
 */
const char * dui_odbc_recordset_get_value (DuiDBRecordSet *, 
                const char * fieldname);

#endif /* USE_ODBC */

#endif /* DUI_ODBC_H */
 
