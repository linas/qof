/********************************************************************\
 * dui-libpg.c -- driver for libpg drivers                        *
 * Copyright (C) 2003,2004 Linas Vepstas <linas@linas.org>               *
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
 * libpg (postrgres) native driver for DUI.  Uses the postgres-dev package
 *
 * HISTORY:
 * Copyright (c) 2003,2004 Linas Vepstas <linas@linas.org>
 */

#if USE_LIBPG_FE

#include <string.h>
#include <time.h>
#include <postgresql/libpq-fe.h>

#include "dui-libpg.h"
#include "dui-initdb-p.h"
#include "perr.h"

/* #include "util.h" */
char * xxxgnc_secs_to_iso8601_buff (time_t secs, char * buff);

/* The following should be eliminated if/when a link to QOF is mandatory */
struct xxxtimespec64
{
   long long int tv_sec;
   long int tv_nsec;
};
typedef struct xxxtimespec64 xxxTimespec;

xxxTimespec xxxgnc_iso8601_to_timespec_gmt(const char *str);


typedef struct DuiLibPGConnection_s DuiLibPGConnection;
typedef struct DuiLibPGRecordSet_s DuiLibPGRecordSet;

struct DuiLibPGConnection_s
{
	DuiDBConnection dbcon;
   char * dbname;
   char * username;
	PGconn  *conn;

	GList * free_pool;
};

struct DuiLibPGRecordSet_s
{
	DuiDBRecordSet recset;
	DuiLibPGConnection *conn;
	PGresult *result;
	long nrows;
	long currow;
};

/* =========================================================== */

DuiDBConnection * 
dui_libpg_connection_new (const char * dbname,
                          const char * username,
                          const char * authentication_token)
{
	PGconn   *conn;
	DuiLibPGConnection *dui_conn;

   conn = PQsetdbLogin ("localhost", NULL, NULL, NULL, 
	                      dbname, username, authentication_token);
	if (!conn) 
	{
		PERR ("Couldn't connect to database with libpg\n");
		return NULL;
	}

	dui_conn = g_new0 (DuiLibPGConnection, 1);
	dui_conn->conn = conn;
	dui_conn->free_pool = NULL;

	dui_conn->dbname = g_strdup (dbname);
	dui_conn->username = g_strdup (username);

	return &dui_conn->dbcon;
}

/* =========================================================== */
static void dui_libpg_recordset_free (DuiLibPGRecordSet *rs);

void
dui_libpg_connection_free (DuiDBConnection *dbc)
{
	GList *node;
	DuiLibPGConnection *conn = (DuiLibPGConnection *)dbc;
	if (!conn) return;

	for (node=conn->free_pool; node; node=node->next)
	{
		dui_libpg_recordset_free (node->data);
	}
	g_list_free (conn->free_pool);
	conn->free_pool = NULL;

	g_free (conn->dbname);
	g_free (conn->username);
	PQfinish (conn->conn);
	conn->conn = NULL;
}

/* =========================================================== */

#define DUIPG_CHECK_CONN(retval)                                       \
   if (!rc) {                                                          \
      const char * ret_str = NULL;                                     \
      ret_str = PQerrorMessage (conn->conn);                           \
      PERR ("Database query execution error:\n%s\n", ret_str);         \
      return retval;                                                   \
   }

#define DUIPG_CHECK_RESULT(retval)                                     \
   if (NULL == result) {                                               \
      char * msg = PQerrorMessage(conn->conn);                         \
      PERR ("Failed to get response from DB: %s\n", msg);              \
      return retval;                                                   \
   }                                                                   \
   ExecStatusType status;                                              \
   status = PQresultStatus(result);                                    \
   if ((PGRES_COMMAND_OK != status) && (PGRES_TUPLES_OK  != status)) { \
      char * msg = PQresultErrorMessage(result);                       \
      PQclear (result);                                                \
      PERR ("Failed to get any results from DB: %s\n", msg);           \
      return retval;                                                   \
   }                                                                   \
   if (0 >= PQntuples (result)) {                                      \
      char * msg = PQresultErrorMessage(result);                       \
      PQclear (result);                                                \
      PERR ("Failed to get any results from DB: %s\n", msg);           \
      return retval;                                                   \
   }


struct timespec 
dui_libpg_get_now (DuiDBConnection *dbc)
{
	struct timespec ts = {0,-1};
	DuiLibPGConnection *conn = (DuiLibPGConnection *) dbc;
	int rc;

	if (!conn) return ts;

	rc = PQsendQuery(conn->conn, "SELECT CURRENT_TIMESTAMP AS qwertyuiopasdfghjkl;");
	DUIPG_CHECK_CONN (ts);

	PGresult *result = PQgetResult (conn->conn);
	DUIPG_CHECK_RESULT (ts);

   const char * val = PQgetvalue (result, 0, 0);
   xxxTimespec xts = xxxgnc_iso8601_to_timespec_gmt (val);
	ts.tv_sec = xts.tv_sec;
	ts.tv_nsec = xts.tv_nsec;
	while (result)
	{
		PQclear(result);
		result = PQgetResult (conn->conn);
	}

	return ts;
}

/* =========================================================== */

void
dui_libpg_lock_table (DuiDBConnection *dbc, const char *table)
{
	DuiLibPGConnection *conn = (DuiLibPGConnection *) dbc;
	if (!conn) return;

	char * str = g_strdup_printf ("LOCK TABLE %s;\n", table);
	int rc = PQsendQuery(conn->conn, str);
	g_free (str);
	DUIPG_CHECK_CONN ( );

	PGresult *result = PQgetResult (conn->conn);
	while (result)
	{
		PQclear(result);
		result = PQgetResult (conn->conn);
	}
}

/* =========================================================== */

void
dui_libpg_unlock_table (DuiDBConnection *dbc, const char *table)
{
	DuiLibPGConnection *conn = (DuiLibPGConnection *) dbc;
	if (!conn) return;

	char * str = g_strdup_printf ("UNLOCK TABLE %s;\n", table);
	int rc = PQsendQuery(conn->conn, str);
	g_free (str);
	DUIPG_CHECK_CONN ( );

	PGresult *result = PQgetResult (conn->conn);
	while (result)
	{
		PQclear(result);
		result = PQgetResult (conn->conn);
	}
}

/* =========================================================== */

DuiDBRecordSet * 
dui_libpg_connection_exec (DuiDBConnection *dbc, const char * buff)
{
	DuiLibPGConnection *conn = (DuiLibPGConnection *) dbc;
	DuiLibPGRecordSet *rs;
	int rc;

	if (!conn) return NULL;

	PINFO ("query=%s", buff);
	rc = PQsendQuery(conn->conn, buff);
	if (!rc) 
	{
		const char * ret_str = NULL;
		ret_str = PQerrorMessage (conn->conn);
		PERR ("Database query execution error:\n%s\n", ret_str);
		return NULL;
	}

	if (conn->free_pool)
	{
		rs = conn->free_pool->data;
		conn->free_pool = g_list_remove (conn->free_pool, rs);
	}
	else
	{
		rs = g_new (DuiLibPGRecordSet, 1);
		rs->conn = conn;
	}

	rs->nrows = -1;
	rs->result = NULL;
	return &rs->recset;
}

/* =========================================================== */

void 
dui_libpg_recordset_release (DuiDBRecordSet *recset)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;
	if (!rs) return;

	PGconn *conn = rs->conn->conn;
	// PQflush (conn);
	// PQconsumeInput (conn);
	if (NULL == rs->result) rs->result = PQgetResult (conn);
	while (rs->result)
	{
		PQclear(rs->result);
		rs->result = PQgetResult (conn);
	}
	rs->conn->free_pool = g_list_prepend (rs->conn->free_pool, rs);
}

/* =========================================================== */

static void 
dui_libpg_recordset_free (DuiLibPGRecordSet *rs)
{
	if (!rs) return;
	g_free (rs);
}

/* =========================================================== */

static void
dui_libpg_fetch_results (DuiDBRecordSet *recset)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;

	/* XXX this is fundamentally wrong, we may need to call
	 * PQgetResult multiple times to get all the rows */

	PGconn *conn = rs->conn->conn;
	rs->result = PQgetResult (conn);
	if (NULL == rs->result)
	{
		char * msg = PQerrorMessage(conn);
		PERR ("Failed to get response from DB: %s\n", msg);
		rs->nrows = 0;
		return;
	}

	ExecStatusType status;
	status = PQresultStatus(rs->result);
	if ((PGRES_COMMAND_OK != status) &&
	    (PGRES_TUPLES_OK  != status))
	{
		char * msg = PQresultErrorMessage(rs->result);
		PQclear (rs->result);
		rs->result = NULL;
		rs->nrows = 0;
		PERR ("Failed to get any results from DB: %s\n", msg);
		return;
	}

	rs->nrows = PQntuples (rs->result);
	rs->currow = -1;
}

/* =========================================================== */

int 
dui_libpg_recordset_rewind (DuiDBRecordSet *recset)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;
	if (!rs) return 0;
	if (0 > rs->nrows && NULL == rs->result)
	{
		dui_libpg_fetch_results (recset);
	}
	rs->currow = 0;
	return rs->nrows;
}

/* =========================================================== */

int
dui_libpg_recordset_fetch_row (DuiDBRecordSet *recset)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;

	if (!rs) return 0;

	if (0 > rs->nrows && NULL == rs->result)
	{
		dui_libpg_fetch_results (recset);
	}

	rs->currow ++;
	if (rs->currow >= rs->nrows) return 0;
	return 1;
}

/* =========================================================== */

const char *
dui_libpg_recordset_get_value (DuiDBRecordSet *recset, const char * fieldname)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;
	const char *val;
	const char *fn;

	if (!rs) return NULL;

	/* The libpq ?????????? can't deal with "tablename.columname" so 
	 * hack around this by hand. */
	fn = strrchr (fieldname, '.');
	if (fn) fn++;
	if (!fn) fn = fieldname;
	if (!fn) return NULL;
	if ('\0' == fn[0]) return NULL;
	
   val = PQgetvalue (rs->result, rs->currow, 
	                  PQfnumber (rs->result, fn));
	return val;
}

/* =========================================================== */

int
dui_libpg_recordset_get_error (DuiDBRecordSet *recset, char **ret_str)
{
	DuiLibPGRecordSet *rs = (DuiLibPGRecordSet *) recset;
	if (!rs) 
	{
		if (ret_str) *ret_str = "Called with null recordset";
		return -1;
	}
#if 0
	pg_error_flag rc = pg_conn_error_flag (rs->conn->conn);
	pg_conn_error (rs->conn->conn, (const char **) ret_str);
	return (int) rc;
#endif
	return 0;
}

/* =========================================================== */

static void
dui_libpg_plugin_free (DuiDBPlugin *plg)
{
	g_free (plg);
}

/* =========================================================== */

static DuiDBPlugin *
dui_libpg_plugin_new (void)
{
	DuiDBPlugin *plg;
	plg = g_new0 (DuiDBPlugin, 1);
	plg->db_provider_name = "libpg";
	plg->plugin_free = dui_libpg_plugin_free;
	plg->connection_new = dui_libpg_connection_new;
	plg->connection_free = dui_libpg_connection_free;
	plg->get_now = dui_libpg_get_now;
	plg->lock = dui_libpg_lock_table;
	plg->unlock = dui_libpg_unlock_table;
	plg->connection_exec = dui_libpg_connection_exec;
	plg->connection_tables = NULL;
	plg->connection_table_columns = NULL;
	plg->recordset_free = dui_libpg_recordset_release;
	plg->recordset_rewind = dui_libpg_recordset_rewind;
	plg->recordset_fetch_row = dui_libpg_recordset_fetch_row;
	plg->recordset_get_value = dui_libpg_recordset_get_value;
	plg->recordset_get_error = dui_libpg_recordset_get_error;

	return plg;
}

/* =========================================================== */

void 
dui_libpg_init (void)
{
	DuiDBPlugin *plg;
	plg = dui_libpg_plugin_new();
	dui_db_provider_register (plg);
}

#endif /* USE_LIBPG_FE */

/* =========================================================== */
