/********************************************************************\
 * dui-initdb.c -- DUI wrapper for different database drivers       *
 * Copyright (C) 2002,2003,2004 Linas Vepstas <linas@linas.org>     *
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
 * Wrapper for different database drivers.
 *
 * XXX fixme: we should make the drivers dlopen thier respective 
 * libraries.
 *
 * HISTORY:
 * Created by Linas Vepstas  March 2002
 * Copyright (c) 2002,2003,2004 Linas Vepstas <linas@linas.org>
 */

#include <dlfcn.h>
#include <string.h>

#include <glib.h>

#include "dui-initdb.h"
#include "dui-initdb-p.h"
#include "perr.h"

/* ========================================================= */

static GList *db_provider_list = NULL;

void 
dui_db_provider_register (DuiDBPlugin *plg)
{
	db_provider_list = g_list_prepend (db_provider_list, plg);
}

static void
load_driver (void *dl_hand, const char * driver, const char * name)
{
	void (*initfn) (void)  = dlsym (dl_hand, driver);
	if (initfn)
	{
		 (*initfn)();
	} 
	else
	{
		const char * err_str = dlerror();
		PERR("Can't load %s Driver, %s\n", name, err_str);
	}
}

void
dui_db_init (void)
{
	void *dl_hand = dlopen ("/home/linas/src/dwi/dwi-cvs/db_drivers/libdwi.so.1.0", RTLD_LAZY);
	if (NULL == dl_hand)
	{
		const char * err_str = dlerror();
		PERR("Can't load DWI DB, %s\n", err_str);
		return;
	}

	load_driver (dl_hand, "dui_odbc_init", "ODBC");
	load_driver (dl_hand, "dui_libpg_init", "Postgres");
	load_driver (dl_hand, "dui_libdbi_init", "DBI");
}

/* ========================================================= */

DuiDBConnection * 
dui_connection_new (const char * provider,
                    const char * dbname, 
                    const char * username,
                    const char * authentication_token)
{
	GList *node;
	DuiDBPlugin *plg = NULL;
	DuiDBConnection *dbc = NULL;

	if (!provider) return NULL;

	for (node=db_provider_list; node; node=node->next)
	{
		DuiDBPlugin *cand = node->data;
		if (!strcmp (provider, cand->db_provider_name)) 
		{
			plg = cand;
			break;
		}
	}
	if (!plg) return NULL;
	if (!plg->connection_new) return NULL;

	dbc = (plg->connection_new) (dbname, username, authentication_token);
	
	if (!dbc) return NULL;
	dbc->provider = plg;
	return dbc;
}

/* ========================================================= */

void 
dui_connection_free (DuiDBConnection *conn)
{
	DuiDBPlugin *plg;
	if (!conn) return;

	plg = conn->provider;
	if (!plg) return;

	
	if (plg->connection_free)
	{
		(plg->connection_free) (conn);
	}
}

/* ========================================================= */

int 
dui_connection_catch_error (DuiDBConnection *conn, char ** ret_str)
{
	return 0;
}
 
/* ========================================================= */

struct timespec
dui_connection_get_now (DuiDBConnection *conn)
{
	struct timespec ts = {0,-1};
	if (!conn) return ts;

	DuiDBPlugin *plg = conn->provider;
	if (!plg) return ts;
	if (!plg->get_now) return ts;

	ts = (plg->get_now) (conn);
	return ts;
}

/* ========================================================= */

void
dui_connection_lock (DuiDBConnection *conn, const char * tablename)
{
	if (!conn) return;

	DuiDBPlugin *plg = conn->provider;
	if (!plg) return;
	if (!plg->lock) return;

	(plg->lock) (conn, tablename);
}

/* ========================================================= */

void
dui_connection_unlock (DuiDBConnection *conn, const char * tablename)
{
	if (!conn) return;

	DuiDBPlugin *plg = conn->provider;
	if (!plg) return;
	if (!plg->unlock) return;

	(plg->unlock) (conn, tablename);
}

/* ========================================================= */

DuiDBRecordSet * 
dui_connection_exec (DuiDBConnection *conn, const char * buff)
{
	DuiDBRecordSet *rs = NULL;
	DuiDBPlugin *plg;
	
	if (!conn) return NULL;

	plg = conn->provider;
	if (!plg) return NULL;
	
	if (plg->connection_exec)
	{
		rs = (plg->connection_exec) (conn, buff);
	}
	if (!rs) return NULL;
	rs->provider = plg;

	return rs;
}

/* ========================================================= */

DuiDBRecordSet * 
dui_connection_tables (DuiDBConnection *conn)
{
	DuiDBRecordSet *rs = NULL;
	DuiDBPlugin *plg;
	
	if (!conn) return NULL;

	plg = conn->provider;
	if (!plg) return NULL;
	
	if (plg->connection_tables)
	{
		rs = (plg->connection_tables) (conn);
	}
	if (!rs) return NULL;
	rs->provider = plg;

	return rs;
}

/* ========================================================= */

DuiDBRecordSet * 
dui_connection_table_columns (DuiDBConnection *conn, const char * tablename)
{
	DuiDBRecordSet *rs = NULL;
	DuiDBPlugin *plg;
	
	if (!conn) return NULL;

	plg = conn->provider;
	if (!plg) return NULL;
	
	if (plg->connection_table_columns)
	{
		rs = (plg->connection_table_columns) (conn, tablename);
	}
	if (!rs) return NULL;
	rs->provider = plg;

	return rs;
}

/* ========================================================= */

void 
dui_recordset_free (DuiDBRecordSet *rs)
{
	DuiDBPlugin *plg;
	
	if (!rs) return;

	plg = rs->provider;
	if (!plg) return;
	
	if (plg->recordset_free)
	{
		(plg->recordset_free) (rs);
	}
}

/* ========================================================= */

int 
dui_recordset_rewind (DuiDBRecordSet *rs)
{
	DuiDBPlugin *plg;
	
	if (!rs) return 0;

	plg = rs->provider;
	if (!plg) return 0;
	
	if (plg->recordset_rewind)
	{
		return (plg->recordset_rewind) (rs);
	}
	return 0;
}

/* ========================================================= */

int 
dui_recordset_fetch_row (DuiDBRecordSet *rs)
{
	DuiDBPlugin *plg;
	
	if (!rs) return 0;

	plg = rs->provider;
	if (!plg) return 0;
	
	if (plg->recordset_fetch_row)
	{
		return (plg->recordset_fetch_row) (rs);
	}
	return 0;
}

/* ========================================================= */

const char * 
dui_recordset_get_value (DuiDBRecordSet *rs, const char * fieldname)
{
	DuiDBPlugin *plg;
	
	if (!rs) return NULL;

	plg = rs->provider;
	if (!plg) return NULL;
	
	if (plg->recordset_get_value)
	{
		return (plg->recordset_get_value) (rs, fieldname);
	}
	return NULL;
}

/* ========================================================= */

int 
dui_recordset_catch_error (DuiDBRecordSet *rs, char ** ret_str)
{
	DuiDBPlugin *plg;
	
	if (!rs) return 0;

	plg = rs->provider;
	if (!plg) return 0;
	
	if (plg->recordset_get_error)
	{
		return (plg->recordset_get_error) (rs, ret_str);
	}
	return 0;
}

/* ==================== END OF FILE ======================== */
