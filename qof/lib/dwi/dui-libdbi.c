/********************************************************************\
 * dui-libdbi.c -- driver for libdbi drivers                        *
 * Copyright (C) 2003 Linas Vepstas <linas@linas.org>               *
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
 * libdbi driver for DUI.  Uses the libdbi.sourceforge.net project
 *
 * HISTORY:
 * Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 */

#if USE_LIBDBI

#include <string.h>
#include <dbi/dbi.h>

#include "dui-libdbi.h"
#include "dui-initdb-p.h"
#include "perr.h"

/* #include "util.h" */
char * xxxgnc_secs_to_iso8601_buff (time_t secs, char * buff);

typedef struct DuiLibDBIConnection_s DuiLibDBIConnection;
typedef struct DuiLibDBIRecordSet_s DuiLibDBIRecordSet;

struct DuiLibDBIConnection_s
{
	DuiDBConnection dbcon;
   char * dbname;
   char * username;
	dbi_driver driver;
	dbi_conn   conn;

	GList * free_pool;
};

struct DuiLibDBIRecordSet_s
{
	DuiDBRecordSet recset;
	DuiLibDBIConnection *conn;
	dbi_result result;
	long long nrows;
};

/* =========================================================== */

DuiDBConnection * 
dui_libdbi_connection_new (const char * dbname,
                          const char * username,
                          const char * authentication_token)
{
	int rc;
	dbi_driver driver;
	dbi_conn   conn;
	const char * errmsg;
	DuiLibDBIConnection *dui_conn;

	rc = dbi_initialize (NULL);
	if (0 >= rc)
	{
		PERR ("Can't load any libdbi drivers, rc=%d\n", rc);
		return NULL;
	}

	/* XXX should parse the dbname to get the driver 
    * e.g. postgres://dbname */
	driver = dbi_driver_list (NULL); 
	driver = dbi_driver_open ("pgsql");
	PINFO ("Using libdbi driver \"%s\"\n", dbi_driver_get_name(driver));
	if (!driver) 
	{
		PERR ("Couldn't load libdbi database driver\n");
		goto shutdown;
	}

	conn = dbi_conn_open (driver);
	conn = dbi_conn_new ("pgsql");
	if (!conn) 
	{
		PERR ("Couldn't connect to database with libdbi\n");
		goto shutdown;
	}

	// rc = dbi_conn_set_option(conn, "host", "localhost");
	rc = dbi_conn_set_option(conn, "username", username);
	// rc = dbi_conn_set_option(conn, "password", authentication_token); 
	rc = dbi_conn_set_option(conn, "dbname", dbname);

	rc = dbi_conn_connect (conn);
	if (rc) 
	{
		dbi_conn_error (conn, &errmsg);
		PERR("Couldn't log into database \"%s\" with username \"%s\":\n"
		     "\terrmsg: %s\n", 
		       dbname, username, errmsg);
		goto disconnect;
	}

	dui_conn = g_new0 (DuiLibDBIConnection, 1);
	dui_conn->driver = driver;
	dui_conn->conn = conn;
	dui_conn->free_pool = NULL;

	dui_conn->dbname = g_strdup (dbname);
	dui_conn->username = g_strdup (username);

	return &dui_conn->dbcon;

disconnect:
	dbi_conn_close (conn);
shutdown:
	dbi_shutdown();
	return NULL;
}

/* =========================================================== */
static void dui_libdbi_recordset_free (DuiLibDBIRecordSet *rs);

void
dui_libdbi_connection_free (DuiDBConnection *dbc)
{
	GList *node;
	DuiLibDBIConnection *conn = (DuiLibDBIConnection *)dbc;
	if (!conn) return;

	for (node=conn->free_pool; node; node=node->next)
	{
		dui_libdbi_recordset_free (node->data);
	}
	g_list_free (conn->free_pool);
	conn->free_pool = NULL;

	g_free (conn->dbname);
	g_free (conn->username);
	dbi_conn_close (conn->conn);
	dbi_shutdown();
}

/* =========================================================== */

DuiDBRecordSet * 
dui_libdbi_connection_exec (DuiDBConnection *dbc, const char * buff)
{
	DuiLibDBIConnection *conn = (DuiLibDBIConnection *) dbc;
	DuiLibDBIRecordSet *rs;
	dbi_result result;

	if (!conn) return NULL;

	PINFO ("query=%s", buff);
	result = dbi_conn_query(conn->conn, buff);
	if (!result) 
	{
		dbi_error_flag flg = dbi_conn_error_flag(conn->conn);
		if (DBI_ERROR_NONE != flg)
		{
			char * ret_str = NULL;
			dbi_conn_error (conn->conn, (const char **) &ret_str);
			PERR ("Database query execution error:\n%s\n", ret_str);
		}
		return NULL;
	}

	if (conn->free_pool)
	{
		rs = conn->free_pool->data;
		conn->free_pool = g_list_remove (conn->free_pool, rs);
	}
	else
	{
		rs = g_new (DuiLibDBIRecordSet, 1);
		rs->conn = conn;
	}

	rs->nrows = -1;
	rs->result = result;
	return &rs->recset;
}

/* =========================================================== */

void 
dui_libdbi_recordset_release (DuiDBRecordSet *recset)
{
	DuiLibDBIRecordSet *rs = (DuiLibDBIRecordSet *) recset;
	if (!rs) return;

	dbi_result_free(rs->result);
	rs->conn->free_pool = g_list_prepend (rs->conn->free_pool, rs);
}

/* =========================================================== */

static void 
dui_libdbi_recordset_free (DuiLibDBIRecordSet *rs)
{
	if (!rs) return;
	g_free (rs);
}

/* =========================================================== */

int 
dui_libdbi_recordset_rewind (DuiDBRecordSet *recset)
{
	DuiLibDBIRecordSet *rs = (DuiLibDBIRecordSet *) recset;
	int rc;
	if (!rs) return 0;
	rc = dbi_result_first_row (rs->result);
	return rc;
}

/* =========================================================== */

int
dui_libdbi_recordset_fetch_row (DuiDBRecordSet *recset)
{
	DuiLibDBIRecordSet *rs = (DuiLibDBIRecordSet *) recset;
	int rc;
	if (!rs) return 0;
	rc = dbi_result_next_row (rs->result);
	return rc;
}

/* =========================================================== */

const char *
dui_libdbi_recordset_get_value (DuiDBRecordSet *recset, const char * fieldname)
{
	DuiLibDBIRecordSet *rs = (DuiLibDBIRecordSet *) recset;
	const char *val;
	const char *fn;

	if (!rs) return NULL;

	/* The libdbi can't deal with "tablename.columname" so 
	 * hack around this by hand. */
	fn = strrchr (fieldname, '.');
	if (fn) fn++;
	if (!fn) fn = fieldname;
	if (!fn) return NULL;
	if ('\0' == fn[0]) return NULL;
	if (0 > rs->nrows)
	{
		rs->nrows = dbi_result_get_numrows (rs->result);
	}
	if (0 >= rs->nrows) return NULL;
	
	unsigned short field_type = dbi_result_get_field_type(rs->result, fn);
	switch (field_type)
	{
		case DBI_TYPE_STRING:
			val = dbi_result_get_string(rs->result, fn);
			return val;

		case DBI_TYPE_INTEGER:
		{
			static char buff[40];       /* XXX not multi-thread safe */
			// unsigned long attr = dbi_result_get_field_attribs (rs->result, fn);

			long lll = dbi_result_get_long(rs->result, fn);
			sprintf (buff, "%ld", lll);
			return buff;
		}
		case DBI_TYPE_DECIMAL:
		{
			static char flbuff[40];       /* XXX not multi-thread safe */
			int fsz = dbi_result_get_field_size (rs->result, fn);
			double ddd = 0.0;
			switch (fsz & ~DBI_DECIMAL_UNSIGNED)
			{
				case 0:
				case DBI_DECIMAL_SIZE4:
					ddd = dbi_result_get_float(rs->result, fn);
					break;
				case DBI_DECIMAL_SIZE8:
					ddd = dbi_result_get_double(rs->result, fn);
					break;
				default:
					PERR ("libdbi database driver:\n"
					      "unhandled float size %x for field=%s", fsz, fieldname);
			}
			sprintf (flbuff, "%25.18g", ddd);
			return flbuff;
		}
		case DBI_TYPE_DATETIME:
		{
			time_t secs = dbi_result_get_datetime (rs->result, fn);
			static char datebuff[140];       /* XXX not multi-thread safe */
			xxxgnc_secs_to_iso8601_buff (secs, datebuff);
			return datebuff;
		}
		case 0:
			PERR("DUI libdbi field \"%s\" is not in the SQL Query\n", 
			     fieldname);
			return NULL;

		default:
			PERR("DUI libdbi field \"%s\" has unsupported field type %d\n", 
			     fieldname, field_type);
			return NULL;
	}

	return val;
}

/* =========================================================== */

int
dui_libdbi_recordset_get_error (DuiDBRecordSet *recset, char **ret_str)
{
	DuiLibDBIRecordSet *rs = (DuiLibDBIRecordSet *) recset;
	if (!rs) 
	{
		if (ret_str) *ret_str = "Called with null recordset";
		return -1;
	}
	dbi_error_flag rc = dbi_conn_error_flag (rs->conn->conn);
	dbi_conn_error (rs->conn->conn, (const char **) ret_str);
	return (int) rc;
}

/* =========================================================== */

static void
dui_libdbi_plugin_free (DuiDBPlugin *plg)
{
	g_free (plg);
}

/* =========================================================== */

static DuiDBPlugin *
dui_libdbi_plugin_new (void)
{
	DuiDBPlugin *plg;
	plg = g_new0 (DuiDBPlugin, 1);
	plg->db_provider_name = "libdbi";
	plg->plugin_free = dui_libdbi_plugin_free;
	plg->connection_new = dui_libdbi_connection_new;
	plg->connection_free = dui_libdbi_connection_free;
	plg->connection_exec = dui_libdbi_connection_exec;
	plg->connection_tables = NULL;
	plg->connection_table_columns = NULL;
	plg->recordset_free = dui_libdbi_recordset_release;
	plg->recordset_rewind = dui_libdbi_recordset_rewind;
	plg->recordset_fetch_row = dui_libdbi_recordset_fetch_row;
	plg->recordset_get_value = dui_libdbi_recordset_get_value;
	plg->recordset_get_error = dui_libdbi_recordset_get_error;

	return plg;
}

/* =========================================================== */

void 
dui_libdbi_init (void)
{
	DuiDBPlugin *plg;
	plg = dui_libdbi_plugin_new();
	dui_db_provider_register (plg);
}

#endif /* USE_LIBDBI */

/* =========================================================== */
