/********************************************************************\
 * database.c -- manage database connection info.                   *
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
 * HISTORY:
 * Linas Vepstas March 2002
 */

#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#include "perr.h"
#include "database.h"
#include "dui-initdb.h"

struct DuiDatabase_s 
{
	GObject gobject;
	char * name;
	
	/* database info */
	char * provider;
	char * dbname;
	char * hostname;
	char * username;
	char * authent;

	/* current database connection that is being used */
	DuiDBConnection *db_conn;
};

/* properties */
enum
{
	DB_NAME=1,
	DB_PROVIDER,
	DB_DBNAME,
	DB_HOSTNAME,
	DB_USERNAME,
	DB_AUTH,
};

typedef struct DuiDatabaseClass_s
{
	GObjectClass parent_class;
} DuiDatabaseClass;

/* =================================================================== */

static void
db_get_property (GObject *object, guint property_id,
                 GValue *value, GParamSpec  *pspec)
{
	char * str = NULL;
	GValue val;
	DuiDatabase *db = DUI_DATABASE(object);
	
	g_value_init (&val, G_TYPE_STRING);
	switch (property_id)
	{
		case DB_NAME:     str=db->name;     break;
		case DB_PROVIDER: str=db->provider; break;
		case DB_DBNAME:   str=db->dbname;   break;
		case DB_HOSTNAME: str=db->hostname; break;
		case DB_USERNAME: str=db->username; break;
		case DB_AUTH:     str=db->authent;  break;
				  
	}
	g_value_set_string (&val, str);
	g_value_copy (&val, value);
}


static void
db_set_property (GObject *object, guint property_id,
                 const GValue *value, GParamSpec  *pspec)
{
	char * str;
	DuiDatabase *db = DUI_DATABASE(object);

	str = g_strdup (g_value_get_string (value));
	switch (property_id)
	{
		case DB_NAME:     g_free (db->name);     db->name = str;     break;
		case DB_PROVIDER: g_free (db->provider); db->provider = str; break;
		case DB_DBNAME:   g_free (db->dbname);   db->dbname = str;   break;
		case DB_HOSTNAME: g_free (db->hostname); db->hostname = str; break;
		case DB_USERNAME: g_free (db->username); db->username = str; break;
		case DB_AUTH:     g_free (db->authent);  db->authent = str;  break;
	}
}


/* =================================================================== */

#define INSTALL(PROP,NAME,DESC) {                                \
	GParamSpec *pspec;                                            \
	pspec = g_param_spec_string (NAME, NULL,                      \
	            DESC, NULL, G_PARAM_READWRITE);                   \
	g_object_class_install_property (goc, PROP, pspec);           \
}


static void
db_class_init (DuiDatabaseClass *dbclass, gpointer class_data)
{
	GObjectClass *goc;

	dbclass->parent_class.set_property = db_set_property;
	dbclass->parent_class.get_property = db_get_property;

	goc = G_OBJECT_CLASS (dbclass);
	
	INSTALL (DB_NAME,     "name",     "DWI Database term name");
	INSTALL (DB_PROVIDER, "provider", "DWI database driver name");
	INSTALL (DB_DBNAME,   "dbname",   "Database name");
	INSTALL (DB_HOSTNAME, "hostname", "TCP/IP host name");
	INSTALL (DB_USERNAME, "username", "User Login");
	INSTALL (DB_AUTH,     "authentication", "Password or Other Authentication");
}


/* =================================================================== */

static void
db_init (DuiDatabase *db, DuiDatabaseClass *dbclass)
{
	db->name = NULL;
	db->provider = NULL;
	db->dbname = NULL;
	db->hostname = NULL;
	db->username = NULL;
	db->authent = NULL;

	db->db_conn = NULL;
}

/* =================================================================== */

GType 
dui_database_get_type (void)
{
	static GType db_type = 0;

	if (!db_type)
	{
		GTypeInfo db_info =
		{
			sizeof (DuiDatabaseClass),
			NULL,
			NULL,
			(GClassInitFunc) db_class_init,
			NULL, NULL,
			sizeof (DuiDatabase),
			0,
			(GInstanceInitFunc) db_init,
			NULL
		};

		db_type = g_type_register_static (G_TYPE_OBJECT, 
			"DuiDatabase", &db_info, 0);
	}
	return db_type;
}

/* =================================================================== */

DuiDatabase *
dui_database_new (const char * name,
                  const char * provider, const char * dbname,
                  const char * hostname, const char * username,
                  const char * passwd)
{
	DuiDatabase *db;

	g_type_init();   /* assume no one else has don this yet */

	db = g_object_new (DUI_DATABASE_TYPE, NULL);

	/* Any of these values might be set later */
	if (!name) name = "";
	if (!provider) provider = "";
	if (!dbname) dbname = "";
	if (!hostname) hostname = "";
	if (!username) username = "";
	if (!passwd) passwd = "";

	db->name     = g_strdup (name);
	db->provider = g_strdup (provider);
	db->dbname   = g_strdup (dbname);
	db->hostname = g_strdup (hostname);
	db->username = g_strdup (username);
	db->authent  = g_strdup (passwd);

	db->db_conn = NULL;

	return db;
}

/* =================================================================== */

void
dui_database_destroy (DuiDatabase *db) 
{
	dui_connection_free (db->db_conn);
	g_free (db->name);
	g_free (db->provider);
	g_free (db->hostname);
	g_free (db->dbname);
	g_free (db->username);
	g_free (db->authent);

	/* hack alert XXX need to free the object itself. Not sure how ... */
}

/* =================================================================== */

DuiDBConnection *
dui_database_do_realize (DuiDatabase *db)
{
	
	if (!db) return NULL;
	if (db->db_conn) return db->db_conn;
	
	/* now actually open the database */
	db->db_conn = dui_connection_new (db->provider, 
	                db->dbname, db->username, db->authent);
	if (NULL == db->db_conn)
	{
		PWARN ("Can't open connection for provider=\"%s\", "
		       "database=\"%s\", username=\"%s\"",
		       db->provider, db->dbname, db->username);
	}
	return db->db_conn;
}

/* =================================================================== */

const char *
dui_database_get_name (DuiDatabase *db) 
{
	if (!db) return NULL;
	return db->name;
}

/* ========================== END OF FILE ============================ */
