/********************************************************************\
 * dui-initdb-p.h - plugin for database drivers                     *
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

#ifndef DUI_INITDB_P_H
#define DUI_INITDB_P_H

#include "dui-initdb.h"

typedef struct DuiDBPlugin_s DuiDBPlugin;

struct DuiDBConnection_s
{
	DuiDBPlugin *provider;
};

struct DuiDBRecordSet_s
{
	DuiDBPlugin *provider;
};

struct DuiDBPlugin_s
{
	const char * db_provider_name;
	void (*plugin_free) (DuiDBPlugin *);
	DuiDBConnection * (*connection_new) (const char * dbname,
	                                     const char * username,
	                                     const char * authentication_token);
	void (*connection_free) (DuiDBConnection *);
	struct timespec (*get_now) (DuiDBConnection *);
	void (*lock) (DuiDBConnection *, const char *);
	void (*unlock) (DuiDBConnection *, const char *);

	DuiDBRecordSet * (*connection_exec) (DuiDBConnection *, const char *);
	DuiDBRecordSet * (*connection_tables) (DuiDBConnection *);
	DuiDBRecordSet * (*connection_table_columns) (DuiDBConnection *, const char *);
	void (*recordset_free) (DuiDBRecordSet *rs);
	int (*recordset_rewind ) (DuiDBRecordSet *);
	int (*recordset_fetch_row) (DuiDBRecordSet *rs);
	const char * (*recordset_get_value) (DuiDBRecordSet *, const char *);
	int (*recordset_get_error) (DuiDBRecordSet *, char **);
};

void dui_db_provider_register (DuiDBPlugin *);

#endif /* DUI_INITDB_P_H */
 
