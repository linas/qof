/********************************************************************\
 * dui-libdpg.h -- driver for the postgres library                  *
 * Copyright (C) 2003,2004 Linas Vepstas <linas@linas.org>          *
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
 * libpq-fe (postgres) driver for DUI.  Uses the postgresql-dev
 * package
 *
 * HISTORY:
 * Copyright (c) 2003,2004 Linas Vepstas <linas@linas.org>
 */

#ifndef DUI_LIBPQ_FE_H_
#define DUI_LIBPQ_FE_H_

#if USE_LIBPQ_FE

#include "dui-initdb.h"

/* Call before using anything else */
void dui_libpq_init (void);

/** establish new connection to indicated database */
DuiDBConnection * dui_libpq_connection_new (const char * dbname,
                                           const char * username,
                                           const char * authentication_token);

void dui_libpq_connection_free (DuiDBConnection *conn);

/** Issue a query on this connection.  FYI -- the query is actually performed
 * asynchronously; the record set may not yet contain valid data when this
 * routine returns.  */
DuiDBRecordSet * dui_libpq_connection_exec (DuiDBConnection *,
                const char * buff);

/** Release the record set when one is done with it */
void dui_libpq_recordset_release (DuiDBRecordSet *);

/** Prep a row.  Basically, this gets the 'next' row out of the database.
 * Subsequent field fetches are for this row. Returns zero if there are
 * no more rows.
 */
int dui_libpq_recordsettch_row (DuiDBRecordSet *);

/** Get the field value by name.  This is a little different/easier than
 * the usual trick of getting it by column number. Slower too.
 */
const char * dui_libpq_recordset_get_value (DuiDBRecordSet *rs,
                                             const char * fieldname);



#endif /* USE_LIBPQ_FE */
#endif /* DUI_LIBPQ_FE_H_ */
