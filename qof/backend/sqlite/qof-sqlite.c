/****************************************************************
 *            qof-sqlite.c
 *
 *  Sun Jan 15 12:52:46 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#define _GNU_SOURCE
#include "config.h"
#include <glib/gstdio.h>
#include <sqlite.h>
#include <glib.h>
#include "qof.h"

#define QOF_MOD_SQLITE "qof-sqlite-module"
#define ACCESS_METHOD "sqlite"

/* Indicates an item with high priority.  */
#define PRIORITY_HIGH       9
/* Indicates an item with default priority. */
#define PRIORITY_STANDARD   5
/* Indicates a low priority item.  */
#define PRIORITY_LOW        0
/* Indicate an error to sqlite */
#define QSQL_ERROR          -1

#define END_DB_VERSION " dbversion int );"

static QofLogModule log_module = QOF_MOD_SQLITE;

typedef enum
{
	SQL_NONE = 0,  /* no operation defined. init value. */
	SQL_CREATE,    /* Create a new database */
	SQL_LOAD,      /* Load all data from existing database. */
	SQL_WRITE,     /* Write / sync all data to the database. */
	SQL_INSERT,    /* Run a single INSERT statement. */
	SQL_DELETE,    /* Run a single DELETE statement. */
	SQL_UPDATE     /* Run a single UPDATE statement. */
}qsql_statement_type;

typedef struct
{
	QofBackend be;
	sqlite *sqliteh;
	qsql_statement_type stm_type;
	gint dbversion;
	const gchar *fullpath;
	gchar *err;
	gchar *sql_str;
	gboolean error;
	QofIdType e_type;
	QofBook * book;
} QSQLiteBackend;

static gchar*
qsql_param_to_sql(QofParam *param)
{
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_STRING))
		return g_strdup_printf(" %s mediumtext", param->param_name);
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_BOOLEAN))
		return g_strdup_printf(" %s int", param->param_name);
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_GUID))
		return g_strdup_printf(" %s char(32) PRIMARY KEY NOT NULL",
			param->param_name);
	if((0 == safe_strcmp(param->param_type, QOF_TYPE_NUMERIC))
		|| (0 == safe_strcmp(param->param_type, QOF_TYPE_DOUBLE))
		|| (0 == safe_strcmp(param->param_type, QOF_TYPE_DEBCRED)))
	{
		return g_strdup_printf(" %s double", param->param_name);
	}
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_INT32))
		return g_strdup_printf(" %s int", param->param_name);
#ifndef QOF_DISABLE_DEPRECATED
	if((0 == safe_strcmp(param->param_type, QOF_TYPE_DATE)) ||
		(0 == safe_strcmp(param->param_type, QOF_TYPE_TIME)))
#else
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_TIME))
#endif
		return g_strdup_printf(" %s datetime", param->param_name);
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_CHAR))
		return g_strdup_printf(" %s char(1)", param->param_name);
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_KVP))
		return g_strdup_printf(" %s mediumtext", param->param_name);
	if(0 == safe_strcmp(param->param_type, QOF_TYPE_COLLECT))
		return g_strdup_printf(" %s char(32)", param->param_name);
	return g_strdup_printf(" %s char(32)", param->param_name);
}

struct qsql_builder
{
	QSQLiteBackend *qsql_be;
	QofEntity *ent;
	gchar *sql_str;
};

static void
create_param_list (QofParam *param, gpointer user_data)
{
	struct qsql_builder *qb;
	qb = (struct qsql_builder*)user_data;

	if (!g_str_has_suffix (qb->sql_str, "("))
		qb->sql_str = g_strconcat (qb->sql_str, ", ", 
			param->param_name, NULL);
	else
		qb->sql_str = g_strconcat (qb->sql_str, 
			param->param_name, NULL);
}

static void
create_each_param (QofParam *param, gpointer user_data)
{
	gchar *value;
	struct qsql_builder *qb;
	qb = (struct qsql_builder*)user_data;

	/** \todo handle KvpFrame */
	value = qof_util_param_to_string (qb->ent, param);
	if (value)
		g_strescape (value, NULL);
	if (!value)
		value = g_strdup ("");
	if (!g_str_has_suffix (qb->sql_str, "("))
		qb->sql_str = g_strconcat (qb->sql_str, ", \"", 
			value, "\"", NULL);
	else
		qb->sql_str = g_strconcat (qb->sql_str, "\"",
			value, "\"", NULL);
}

/* need new-style event handlers for insert and update 
insert runs after QOF_EVENT_CREATE
delete runs before QOF_EVENT_DESTROY
*/
static void 
delete_event (QofEntity *ent, QofEventId event_type, 
			  gpointer handler_data, gpointer event_data)
{
	QofBackend *be;
	QSQLiteBackend *qsql_be;
	gchar gstr[GUID_ENCODING_LENGTH+1];
	gchar * sql_str;

	qsql_be = (QSQLiteBackend*)handler_data;
	be = (QofBackend*)qsql_be;
	if (!ent)
		return;
	if (0 == safe_strcmp (ent->e_type, QOF_ID_BOOK))
		return;
	ENTER (" %s", ent->e_type);
	switch (event_type)
	{
		case QOF_EVENT_DESTROY :
		{
			guid_to_string_buff (qof_entity_get_guid (ent), gstr);
			sql_str = g_strdup_printf ("DELETE from %s ", 
				ent->e_type);
			sql_str = g_strconcat (sql_str, 
				"WHERE ", QOF_TYPE_GUID, "='", gstr,
				"';", NULL);
			DEBUG (" sql_str=%s", sql_str);
			if(sqlite_exec (qsql_be->sqliteh, sql_str, 
				NULL, qsql_be, &qsql_be->err) !=
				SQLITE_OK)
			{
				qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
				qsql_be->error = TRUE;
				PERR (" %s", qsql_be->err);
			}
			break;
		}
		default : break;
	}
	LEAVE (" ");	
}

static void 
create_event (QofEntity *ent, QofEventId event_type, 
			  gpointer handler_data, gpointer event_data)
{
	QofBackend *be;
	struct qsql_builder qb;
	QSQLiteBackend *qsql_be;

	qsql_be = (QSQLiteBackend*)handler_data;
	be = (QofBackend*)qsql_be;
	if (!ent)
		return;
	if (0 == safe_strcmp (ent->e_type, QOF_ID_BOOK))
		return;
	ENTER (" %s", ent->e_type);
	switch (event_type)
	{
		case QOF_EVENT_CREATE :
		{
			qb.ent = ent;
			qb.sql_str = g_strdup_printf ("INSERT into %s (", 
				ent->e_type);
			qof_class_param_foreach (ent->e_type, create_param_list, 
				&qb);
			qb.sql_str = g_strconcat (qb.sql_str, ") VALUES (", NULL);
			qof_class_param_foreach (ent->e_type, 
				create_each_param, &qb);
			qb.sql_str = g_strconcat (qb.sql_str, ");", NULL);
			DEBUG (" sql_str=%s", qb.sql_str);
			if(sqlite_exec (qsql_be->sqliteh, qb.sql_str, 
				NULL, qsql_be, &qsql_be->err) !=
				SQLITE_OK)
			{
				qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
				qsql_be->error = TRUE;
				PERR (" %s", qsql_be->err);
			}
			else
				((QofInstance*)ent)->dirty = FALSE;
			break;
		}
		default : break;
	}
	LEAVE (" ");
}

static void
qsql_modify (QofBackend *be, QofInstance *inst)
{
	struct qsql_builder qb;
	QSQLiteBackend *qsql_be;
	gchar gstr[GUID_ENCODING_LENGTH+1];
	gchar * param_str;

	qsql_be = (QSQLiteBackend*)be;
	if (!inst)
		return;
	if (!inst->param)
		return;
	ENTER (" ");

	guid_to_string_buff (qof_instance_get_guid (inst), gstr);
	qb.ent = (QofEntity*)inst;
	param_str = qof_util_param_to_string (qb.ent, inst->param);
	g_strescape (param_str, NULL);
	qb.sql_str = g_strdup_printf ("UPDATE %s SET %s=\"",
		qb.ent->e_type, inst->param->param_name);
	qb.sql_str = g_strconcat (qb.sql_str, param_str, 
		"\" WHERE ", QOF_TYPE_GUID, "='", gstr,
		"';", NULL);
	DEBUG (" sql_str=%s param_Str=%s", qb.sql_str, param_str);
	if(sqlite_exec (qsql_be->sqliteh, qb.sql_str, 
		NULL, qsql_be, &qsql_be->err) !=
		SQLITE_OK)
	{
		qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
		qsql_be->error = TRUE;
		PERR (" %s", qsql_be->err);
	}
	else
		inst->dirty = FALSE;
	LEAVE (" ");
}

static gint 
qsql_record_foreach(gpointer data, gint col_num, gchar **strings,
					gchar **columnNames)
{
	QSQLiteBackend *qsql_be;
	const QofParam * param;
	QofInstance * inst;
	gint i;

	g_return_val_if_fail(data, QSQL_ERROR);
	qsql_be = (QSQLiteBackend*)data;
	qof_event_suspend ();
	inst = (QofInstance*)qof_object_new_instance (qsql_be->e_type,
		qsql_be->book);
	ENTER (" loading %s", qsql_be->e_type);
	for(i = 0;i < col_num; i++)
	{
		/* get param and set as string */
		param = qof_class_get_parameter (qsql_be->e_type, 
			columnNames[i]);
		if (!param)
			continue;
		if (0 == safe_strcmp (columnNames[i], QOF_TYPE_GUID))
		{
			GUID * guid;
			guid = guid_malloc();
			if (!string_to_guid(strings[i], guid))
			{
				DEBUG (" set guid failed:%s", strings[i]);
				return QSQL_ERROR;
			}
			qof_entity_set_guid (&inst->entity, guid);
		}
		if (strings[1])
			qof_util_param_set_string (&inst->entity, param, strings[i]);
	}
	qof_event_resume ();
	LEAVE (" ");
	return SQLITE_OK;
}

static void
qsql_param_foreach(QofParam *param, gpointer data)
{
	QSQLiteBackend *qsql_be;
	gchar *p_str;

	qsql_be = (QSQLiteBackend*)data;
	p_str = qsql_param_to_sql(param);
	qsql_be->sql_str = g_strconcat(qsql_be->sql_str, 
		p_str, ",", NULL);
	g_free(p_str);
}

static gint 
qsql_update_foreach (gpointer data, gint col_num, gchar **strings,
					gchar **columnNames)
{
	const QofParam * param;
	QofInstance * inst;
	struct qsql_builder * qb;
	QSQLiteBackend *qsql_be;
	QofBackend * be;
	gchar gstr[GUID_ENCODING_LENGTH+1];
	gchar * param_str;
	gint i;

	qb = (struct qsql_builder*) data;
	qsql_be = qb->qsql_be;
	be = (QofBackend*)qsql_be;
	inst = (QofInstance*) qb->ent;
	if (!inst->dirty)
		return SQLITE_OK;
	ENTER (" ");
	guid_to_string_buff (qof_entity_get_guid (qb->ent), gstr);
	for(i = 0;i < col_num; i++)
	{
		param = qof_class_get_parameter (qb->ent->e_type, 
			columnNames[i]);
		if (!param)
			continue;
		param_str = qof_util_param_to_string (qb->ent, param);
		g_strescape (param_str, NULL);
		qb->sql_str = g_strdup_printf ("UPDATE %s SET %s=\"",
			qb->ent->e_type, param->param_name);
		qb->sql_str = g_strconcat (qb->sql_str, param_str, 
			"\" WHERE ", QOF_TYPE_GUID, "='", gstr, "';", NULL);
		DEBUG (" sql_str=%s param_str=%s", qb->sql_str, param_str);
		if(sqlite_exec (qsql_be->sqliteh, qb->sql_str, 
			NULL, qsql_be, &qsql_be->err) != SQLITE_OK)
		{
			qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
			qsql_be->error = TRUE;
			PERR (" %s", qsql_be->err);
		}
		else
			inst->dirty = FALSE;
	}
	LEAVE (" ");
	return SQLITE_OK;
}

static void
check_state (QofEntity * ent, gpointer user_data)
{
	gchar gstr[GUID_ENCODING_LENGTH+1];
	QSQLiteBackend *qsql_be;
	struct qsql_builder qb;
	QofBackend *be;

	qsql_be = (QSQLiteBackend*) user_data;
	be = (QofBackend*)qsql_be;
	/* check if this entity already exists */
	guid_to_string_buff (qof_entity_get_guid (ent), gstr);
	qsql_be->sql_str = g_strdup_printf(
		"SELECT * FROM %s where guid = \"%s\";", ent->e_type, gstr);
	PINFO (" write: %s", qsql_be->sql_str);
	qb.ent = ent;
	/* update each dirty instance */
	if(sqlite_exec (qsql_be->sqliteh, qsql_be->sql_str, 
		qsql_update_foreach, &qb, &qsql_be->err) !=
		SQLITE_OK)
	{
		qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
		qsql_be->error = TRUE;
		PERR (" %s", qsql_be->err);
	}
	else
	{
		/* create new entity */
		g_free (qsql_be->sql_str);
		qb.sql_str = g_strdup_printf ("INSERT into %s (", 
			ent->e_type);
		qof_class_param_foreach (ent->e_type, 
			create_param_list, &qb);
		qb.sql_str = g_strconcat (qb.sql_str, ") VALUES (", NULL);
		qof_class_param_foreach (ent->e_type, 
			create_each_param, &qb);
		qb.sql_str = g_strconcat (qb.sql_str, ");", NULL);
		DEBUG (" sql_str= %s", qb.sql_str);
		if(sqlite_exec (qsql_be->sqliteh, qb.sql_str, 
			NULL, qsql_be, &qsql_be->err) != SQLITE_OK)
		{
			qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
			qsql_be->error = TRUE;
			PERR (" %s", qsql_be->err);
		}
	}
}

static void
qsql_class_foreach(QofObject *obj, gpointer data)
{
	QSQLiteBackend *qsql_be;
	QofBackend *be;

	qsql_be = (QSQLiteBackend*)data;
	be = (QofBackend*)qsql_be;
	qsql_be->e_type = obj->e_type;
	ENTER (" obj_type=%s", qsql_be->e_type);
	switch (qsql_be->stm_type)
	{
		case SQL_NONE :
		case SQL_INSERT :
		case SQL_DELETE :
		case SQL_UPDATE :
		{
			break;
		}
		case SQL_CREATE :
		{
			qsql_be->sql_str = g_strdup_printf(
				"CREATE TABLE %s (", obj->e_type);
			qof_class_param_foreach(obj->e_type, 
				qsql_param_foreach, data);
			qsql_be->sql_str = g_strconcat(qsql_be->sql_str,
				END_DB_VERSION, NULL);
			if(sqlite_exec (qsql_be->sqliteh, qsql_be->sql_str, 
				NULL, NULL, &qsql_be->err) != SQLITE_OK)
			{
				qof_backend_set_error(be, ERR_BACKEND_DATA_CORRUPT);
				qsql_be->error = TRUE;
				PERR (" %s", qsql_be->err);
			}
			g_free(qsql_be->sql_str);
			break;
		}
		case SQL_LOAD :
		{
			qsql_be->sql_str = g_strdup_printf(
				"SELECT * FROM %s;", obj->e_type);
			PINFO (" sql=%s", qsql_be->sql_str);
			if(sqlite_exec(qsql_be->sqliteh, qsql_be->sql_str, 
				qsql_record_foreach, qsql_be, &qsql_be->err) !=
					SQLITE_OK)
			{
				qof_backend_set_error(be, ERR_BACKEND_SERVER_ERR);
				qsql_be->error = TRUE;
				PERR (" %s", qsql_be->err);
			}
			break;
		}
		case SQL_WRITE : 
		{
			if (!qof_book_not_saved (qsql_be->book))
				break;
			/* not all objects generate a create event
			when the entity is created. */
			qof_object_foreach (obj->e_type, qsql_be->book,
				check_state, qsql_be);
			break;
		}
	}
	LEAVE (" ");
}

static void
qsql_backend_createdb(QofBackend *be, QofSession *session)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail(be || session);
	ENTER (" ");
	qsql_be = (QSQLiteBackend*)be;
	qsql_be->stm_type = SQL_CREATE;
	qsql_be->book = qof_session_get_book (session);
	qsql_be->sqliteh = sqlite_open (qsql_be->fullpath, 0, 
		&qsql_be->err);
	if(!qsql_be->sqliteh)
	{
		qof_backend_set_error(be, ERR_BACKEND_CANT_CONNECT);
		qsql_be->error = TRUE;
		PERR (" %s", qsql_be->err);
		LEAVE (" ");
		return;
	}
	qof_object_foreach_type(qsql_class_foreach, qsql_be);
	PINFO(" %s", qsql_be->sql_str);
	LEAVE (" ");
}

static void
qsql_backend_opendb (QofBackend *be, QofSession *session)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail(be || session);
	ENTER (" ");
	qsql_be = (QSQLiteBackend*)be;
	qsql_be->sqliteh = sqlite_open (qsql_be->fullpath, 0, 
		&qsql_be->err);
	if(!qsql_be->sqliteh)
	{
		qof_backend_set_error(be, ERR_BACKEND_CANT_CONNECT);
		qsql_be->error = TRUE;
		PERR (" %s", qsql_be->err);
	}
	LEAVE (" ");
}

static void
qsqlite_session_begin(QofBackend *be, QofSession *session, const 
					  gchar *book_path, gboolean ignore_lock,
					  gboolean create_if_nonexistent)
{
	QSQLiteBackend *qsql_be;
	gchar** pp;
	struct stat statinfo;
	gint stat_val;

	g_return_if_fail(be);
	ENTER (" book_path=%s", book_path);
	qsql_be = (QSQLiteBackend*)be;
	qsql_be->fullpath = NULL;
	be->fullpath = g_strdup (book_path);
	if(book_path == NULL)
	{
		qof_backend_set_error(be, ERR_BACKEND_BAD_URL);
		qsql_be->error = TRUE;
		LEAVE (" bad URL");
		return;
	}
	/* book_path => sqlite_file_name */
	pp = g_strsplit(book_path, ":", 2);
	if(0 == safe_strcmp(pp[0], ACCESS_METHOD))
	{
		qsql_be->fullpath = g_strdup(pp[1]);
		g_strfreev (pp);
	}
	stat_val = g_stat (qsql_be->fullpath, &statinfo);
	if (!S_ISREG (statinfo.st_mode) || statinfo.st_size == 0)
		qsql_backend_createdb (be, session);
	qsql_backend_opendb (be, session);
	if(qsql_be->error) 
	{ 
		LEAVE(" open failed"); 
		return; 
	}
	qof_backend_set_error(be, ERR_BACKEND_NO_ERR);
	qof_event_register_handler (create_event, qsql_be);
	qof_event_register_handler (delete_event, qsql_be);
	LEAVE (" db=%s", qsql_be->fullpath);
}

static void
qsqlite_db_load (QofBackend *be, QofBook *book)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail(be);
	ENTER (" ");
	qsql_be = (QSQLiteBackend*)be;
	qsql_be->stm_type = SQL_LOAD;
	qsql_be->book = book;
	/* iterate over registered objects */
	qof_object_foreach_type(qsql_class_foreach, qsql_be);
	LEAVE (" ");
}

static void
qsqlite_write_db (QofBackend *be, QofBook *book)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail(be);
	qsql_be = (QSQLiteBackend*)be;
	qsql_be->stm_type = SQL_WRITE;
	qsql_be->book = book;
	/* update each record with current state */
	qof_object_foreach_type(qsql_class_foreach, qsql_be);
}

static gboolean
qsql_determine_file_type (const gchar *path)
{
	return TRUE;
}

static void
qsqlite_session_end (QofBackend *be)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail(be);
	qsql_be = (QSQLiteBackend*)be;
	if (qsql_be->sqliteh)
		sqlite_close (qsql_be->sqliteh);
}

static void
qsqlite_destroy_backend (QofBackend *be)
{
	g_free (be);
}

static void
qsql_provider_free (QofBackendProvider *prov)
{
	prov->provider_name = NULL;
	prov->access_method = NULL;
	g_free (prov);
}

static QofBackend*
qsql_backend_new(void)
{
	QSQLiteBackend *qsql_be;
	QofBackend *be;

	ENTER (" ");
	qsql_be = g_new0(QSQLiteBackend, 1);
	be = (QofBackend*) qsql_be;
	qof_backend_init(be);
	qsql_be->dbversion = QOF_OBJECT_VERSION;
	qsql_be->stm_type = SQL_NONE;
	be->session_begin = qsqlite_session_begin;

	be->session_end = qsqlite_session_end;
	be->destroy_backend = qsqlite_destroy_backend;
	be->load = qsqlite_db_load;
	be->save_may_clobber_data = NULL;
	be->begin = NULL;
	/* commit: write to sqlite, commit undo record. */
	be->commit = qsql_modify;
	be->rollback = NULL;
	/* would need a QofQuery back to QofSqlQuery conversion. */
	be->compile_query = NULL;
	/* unused */
	be->free_query = NULL;
	be->run_query = NULL;
	be->counter = NULL;
	/* The QOF SQLite backend is not multi-user - all QOF users are the same. */
	be->events_pending = NULL;
	be->process_events = NULL;

	be->sync = qsqlite_write_db;
	be->load_config = NULL;
	be->get_config = NULL;
	LEAVE (" ");
	return be;
}

void qof_sqlite_provider_init(void)
{
	QofBackendProvider *prov;

	ENTER (" ");
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "QOF SQLite Backend Version 0.1";
	prov->access_method = ACCESS_METHOD;
	prov->partial_book_supported = TRUE;
	prov->backend_new = qsql_backend_new;
	prov->check_data_type = qsql_determine_file_type;
	prov->provider_free = qsql_provider_free;
	qof_backend_register_provider (prov);
	LEAVE (" ");
}

/* ================= END OF FILE =================== */
