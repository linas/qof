/********************************************************************\
 * dui-initdb.h - wrapper for different database drivers            *
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
 * @file dui-initdb.h
 * @brief Simplified abstraction layer for database portability
 * @author Copyright (c) 2002,2004 Linas Vepstas <linas@linas.org>
 *
 * This module allows DWI to access a variety of data sources using a uniform
 * programming interface: essentially, this is a database API wrapper.  This
 * wrapper is very DWI-specific, and is not (really) meant for use by other 
 * applications (although it has been).  The API borrows conceptual elements
 * from the Postgres libpg API and the ODBC API, and so should be "inutitively
 * obvious" to experienced database programmers.   This wrapper allows a variety
 * of different database-access drivers to be operated behind it. 
 * 
 * The idea of having a 'wrapper' for database access is not new, and so its 
 * worth discussing why 'yet another' wrapper has been created.  There are
 * several reasons:
 * 
 * 1) This wrapper originially wrapped ODBC.  I didn't know ODBC very well
 *    when I did this, and I was worried that ODBC sucked.  The wrapper 
 *    allows suckage to be hidden or fixed.  
 * 2) The wrapper would allows native Postgress interfaces to be quickly
 *    developed, in case the suckage of 1 was too great.
 * 3) The wrapper allows drivers to be added for non-traditional data sources.
 *    This would allow DWI to treat these non-traditional data sources in a
 *    a database-like way: an important feature, since, to DWI, everything
 *    looks like a database.  Note: to do this properly, we would need to 
 *    move the "builder.h" and "builder.c" files from the other directory
 *    to here: the builder would be used only by SQL data sources; other
 *    data sources would have to handle key-value pairs differently.
 *    
 * This wrapper currently supports only two "providers": ODBC and 
 * libdbi (http://libdbi.sourceforge.net).
 */


#ifndef DUI_INITDB_H
#define DUI_INITDB_H

#include <time.h>

typedef struct DuiDBConnection_s DuiDBConnection;
typedef struct DuiDBRecordSet_s  DuiDBRecordSet;

/** Must call this before using this subsystem */
void dui_db_init(void);

/** The dui_connection_new() routine establishes a connection to
 *    the indicated database, using the indicated login.  The
 *    "provider" field is the name of the driver to use. 
 *    Currently, only "odbc" and "libdbi" are supported.
 *
 *    The ODBC driver expects "dbname" to specify an existing
 *    database, and "username" to specify a valid username for
 *    logging into the database.  The "authentication token"
 *    must be a valid password or other ticket that authorizes
 *    access.
 */
DuiDBConnection * dui_connection_new (const char * provider,
                                      const char * dbname, 
                                      const char * username,
                                      const char * authentication_token);

void dui_connection_free (DuiDBConnection *conn);


/** Issue a query on this connection.  FYI -- the query is actually performed
 * asynchronously; the record set may not yet contain valid data when this 
 * routine returns.  Valid data is not available until the (possibly blocking)
 * routine dui_recordset_fetch_row() is called.  Note that some queries
 * may result in errors: e.g. if a query uses a table that does not exist.
 * If the resulting recordset contains no rows, you may want to check 
 * dui_recordset_catch_error() for errors.
 * */
DuiDBRecordSet * dui_connection_exec (DuiDBConnection *, const char * buff);

/** Return a list of tables on this connection. */
DuiDBRecordSet * dui_connection_tables (DuiDBConnection *);

/** Get a list of columns in the named table */
DuiDBRecordSet * dui_connection_table_columns (DuiDBConnection *,
                      const char * table_name);


/** Release the record set when one is done with it */
void dui_recordset_free (DuiDBRecordSet *);

/** Set the row pointer to the start of the the recordset.  
 *  Returns 0 if there is no first row.
 */
int dui_recordset_rewind (DuiDBRecordSet *rs);

/** Prep a row.  Basically, this gets the 'next' row out of the database.
 *  Subsequent field fetches are for this row. Returns zero if there are 
 *  no more rows, else returns a non-zero value if there are more rows.
 */
int dui_recordset_fetch_row (DuiDBRecordSet *rs);

/** Get the field value by name.  This is a little different/easier than 
 * the usual trick of getting it by column number.
 */
const char * dui_recordset_get_value (DuiDBRecordSet *, const char * fieldname);

/**
 *  Return an error code and error string.   The design philosphy of 
 *  the DUI database connection is to be as self-healing as possible,
 *  and to report as few errors as possible, so that the system does 
 *  'the right thing' as often as possible, so that the API remains 
 *  easy to use. However, its impossible to ignore all errors; this routine
 *  provides a way of getting them. 
 *
 *  Returns zero if no error, else returns non-zero.  An error string
 *  is returned in ret_str if its non-null.  The act of calling this
 *  routine will also clear the error report (but not the error condition).
 */
int dui_connection_catch_error (DuiDBConnection *conn, char ** ret_str);
int dui_recordset_catch_error (DuiDBRecordSet *, char ** ret_str);

/**
 * Return the the current time, in UTC, at the database server. 
 * This value is typically used to provide timestamps for data records.
 * This function is provided as a callable subroutine to ensure 
 * portability and common operation for different databases.  The
 * SQL statement 'SELECT now() as x' is usally enough for most, but
 * possibly not all databases.
 */
struct timespec dui_connection_get_now (DuiDBConnection *conn);

/**
 * Lock the indicated tablename in the database server, thus serializing
 * update access.  This function is provided as a callable subroutine to 
 * ensure portability and common operation for different databases.
 * Not all databases support the same locking mechanisms or locking
 * primitives; this subroutine provides the common api to sereialize
 * database updates.
 *
 * The subroutine takes a tablename to be locked as the argument.
 * The lock is not expected to fail; if the lock can't be gotten
 * immediately, the subroutine will stall until the lock is available.
 * The unlock is expected to globally commit the changes, making them
 * visible to other clients. 
 */
void dui_connection_lock (DuiDBConnection *conn, const char *tablename);

/** XXX under what cases would we need to support rollback? */
void dui_connection_unlock (DuiDBConnection *conn, const char *tablename);

#endif /* DUI_INITDB_H */
 
