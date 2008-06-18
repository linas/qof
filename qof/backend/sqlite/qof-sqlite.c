/****************************************************************
 *            qof-sqlite.c
 *
 *  Sun Jan 15 12:52:46 2006
 *  Copyright  2006-2008  Neil Williams
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

#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <glib/gstdio.h>
#include <sqlite.h>
#include <glib.h>
#include <libintl.h>
#include "qof.h"
#include "qofsql-p.h"
#include "qof-sqlite.h"
#include "kvputil-p.h"

#define _(String) dgettext (GETTEXT_PACKAGE, String)
#define ACCESS_METHOD "sqlite"

/** @file  qof-sqlite.c
	@brief Public interface of qof-backend-sqlite
	@author Copyright 2006-2008 Neil Williams <linux@codehelp.co.uk>
*/

/** Indicates an item with high priority.  */
#define PRIORITY_HIGH       9
/** Indicates an item with default priority. */
#define PRIORITY_STANDARD   5
/** Indicates a low priority item.  */
#define PRIORITY_LOW        0
/** Indicate an error to sqlite */
#define QSQL_ERROR          -1
/** One KVP table per file for all instances.  */
#undef QSQL_KVP_TABLE
#define QSQL_KVP_TABLE "sqlite_kvp"

#define END_DB_VERSION " dbversion int );"

static QofLogModule log_module = QOF_MOD_SQLITE;
static gboolean loading = FALSE;

/** \brief Main context for sqlite backend

Contains data elements that persist for longer
than a single operation or query, generally for the
life of the process.
*/
typedef struct
{
	QofBackend be;
	sqlite *sqliteh;
	QsqlStatementType stm_type;
	gint dbversion;
	gint create_handler;
	gint delete_handler;
	const gchar *fullpath;
	gchar *err;
	gboolean error;
	/* full hashtable of kvp records */
	GHashTable *kvp_table;
	/* hashtable relating the GUID to the kvp_id */
	GHashTable *kvp_id;
	/* highest kvp_id in the table */
	gulong index;
	QofBook *book;
	QofErrorId err_delete, err_insert, err_update, err_create;
} QSQLiteBackend;

/** \brief QOF SQLite context

Used to correlate the sqlite data with the
live data. Contains mainly transient data
for the current entity or the current operation.
*/
struct QsqlBuilder
{
	/** the current sqlite backend */
	QSQLiteBackend *qsql_be;
	/** the current entity */
	QofEntity *ent;
	/** the type of the current entity */
	QofIdType e_type;
	/** the SQL string in use */
	gchar *sql_str;
	/** list of other dirty entities */
	GList *dirty_list;
	/** whether to use UPDATE or INSERT */
	gboolean exists;
	/** ignore an empty KvpFrame */
	gboolean has_slots;
	/** which parameter needs updating in sqlite. */
	const QofParam *dirty;
};

/** \todo reconcile the duplication with the QSF (and GDA) version */
static KvpValue *
string_to_kvp_value (const gchar * content, KvpValueType type)
{
	gchar *tail;
	gint64 cm_i64;
	gdouble cm_double;
	QofNumeric cm_numeric;
	GUID *cm_guid;

	switch (type)
	{
	case KVP_TYPE_GINT64:
		{
			errno = 0;
			cm_i64 = strtoll (content, &tail, 0);
			if (errno == 0)
			{
				return kvp_value_new_gint64 (cm_i64);
			}
			break;
		}
	case KVP_TYPE_DOUBLE:
		{
			errno = 0;
			cm_double = strtod (content, &tail);
			if (errno == 0)
				return kvp_value_new_double (cm_double);
			break;
		}
	case KVP_TYPE_NUMERIC:
		{
			qof_numeric_from_string (content, &cm_numeric);
			return kvp_value_new_numeric (cm_numeric);
			break;
		}
	case KVP_TYPE_STRING:
		{
			return kvp_value_new_string (content);
			break;
		}
	case KVP_TYPE_GUID:
		{
			cm_guid = g_new0 (GUID, 1);
			if (TRUE == string_to_guid (content, cm_guid))
				return kvp_value_new_guid (cm_guid);
			break;
		}
	case KVP_TYPE_TIME:
		{
			QofDate *qd;
			QofTime *qt;
			KvpValue *retval;

			qd = qof_date_parse (content, QOF_DATE_FORMAT_UTC);
			if (qd)
			{
				qt = qof_date_to_qtime (qd);
				retval = kvp_value_new_time (qt);
				qof_date_free (qd);
				qof_time_free (qt);
				return retval;
			}
			else
				PERR (" failed to parse date");
		}
	case KVP_TYPE_BOOLEAN:
		{
			gboolean val;
			val = qof_util_bool_to_int (content);
			return kvp_value_new_boolean (val);
		}
	default:
		break;
	}
	return NULL;
}

/** returns the VALUES for INSERT in pre-defined order */
static G_GNUC_UNUSED void
kvpvalue_to_sql (const gchar * key, KvpValue * val, gpointer builder)
{
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder *qb;
	KvpValueType n;
	gchar *full_path;

	full_path = NULL;
	ENTER (" ");
	qb = (struct QsqlBuilder *) builder;
	qsql_be = qb->qsql_be;
	g_return_if_fail (key && val && qsql_be);
	n = kvp_value_get_type (val);
	switch (n)
	{
	case KVP_TYPE_GINT64:
	case KVP_TYPE_DOUBLE:
	case KVP_TYPE_NUMERIC:
	case KVP_TYPE_STRING:
	case KVP_TYPE_GUID:
	case KVP_TYPE_TIME:
	case KVP_TYPE_BOOLEAN:
		{
			/* ("kvp_id int primary key not null", "guid char(32)", "path mediumtext",
			   "type mediumtext", "value text", */

			qb->sql_str =
				g_strdup_printf (" kvp key=%s val=%s type=%s", key,
				kvp_value_to_bare_string (val),
				kvp_value_type_to_qof_id (n));
			DEBUG (" %s", qb->sql_str);
			qb->has_slots = TRUE;
			break;
		}
	case KVP_TYPE_FRAME:
		{
			kvp_frame_for_each_slot (kvp_value_get_frame (val),
				kvpvalue_to_sql, qb);
			break;
		}
	default:
		{
			PERR (" unsupported value = %d", kvp_value_get_type (val));
			break;
		}
	}
	LEAVE (" %s", qb->sql_str);
}

/** \brief use the new-style event handlers for insert and update
insert runs after QOF_EVENT_CREATE
delete runs before QOF_EVENT_DESTROY
*/
static void
delete_event (QofEntity * ent, QofEventId event_type,
	gpointer handler_data, gpointer event_data)
{
	QofBackend *be;
	QSQLiteBackend *qsql_be;
	gchar *gstr, *sql_str;

	qsql_be = (QSQLiteBackend *) handler_data;
	be = (QofBackend *) qsql_be;
	if (!ent)
		return;
	if (0 == safe_strcmp (ent->e_type, QOF_ID_BOOK))
		return;
	/* do not try to delete if only a QofObject has been loaded. */
	if (!qof_class_is_registered (ent->e_type))
		return;
	switch (event_type)
	{
	case QOF_EVENT_DESTROY:
		{
			ENTER (" %s do_free=%d", ent->e_type,
				((QofInstance *) ent)->do_free);
			gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
			guid_to_string_buff (qof_entity_get_guid (ent), gstr);
			sql_str = qof_sql_entity_delete (ent);
			DEBUG (" sql_str=%s", sql_str);
			if (sqlite_exec (qsql_be->sqliteh, sql_str,
					NULL, qsql_be, &qsql_be->err) != SQLITE_OK)
			{
				qof_error_set_be (be, qsql_be->err_delete);
				qsql_be->error = TRUE;
				LEAVE (" error on delete:%s", qsql_be->err);
				break;
			}
			LEAVE (" %d", event_type);
			qsql_be->error = FALSE;
			g_free (gstr);
			break;
		}
	default:
		break;
	}
}

/** receives QSQLiteBackend, passes on QsqlBuilder */
static void
create_event (QofEntity * ent, QofEventId event_type,
	gpointer handler_data, gpointer event_data)
{
	QofBackend *be;
	struct QsqlBuilder qb;
	QSQLiteBackend *qsql_be;
	gchar *gstr;
	KvpFrame *slots;

	qsql_be = (QSQLiteBackend *) handler_data;
	be = (QofBackend *) qsql_be;
	if (!ent)
		return;
	if (0 == safe_strcmp (ent->e_type, QOF_ID_BOOK))
		return;
	if (!qof_class_is_registered (ent->e_type))
		return;
	switch (event_type)
	{
	case QOF_EVENT_CREATE:
		{
			ENTER (" insert:%s", ent->e_type);
			gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
			guid_to_string_buff (qof_instance_get_guid ((QofInstance *)
					ent), gstr);
			DEBUG (" guid=%s", gstr);
			qb.ent = ent;
			qb.sql_str = qof_sql_entity_insert (ent);
			/** \bug create one func to create_param_list and create_each_param */
			DEBUG (" sql_str=%s", qb.sql_str);
			if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
				NULL, &qb, &qsql_be->err) != SQLITE_OK)
			{
				qof_error_set_be (be, qsql_be->err_insert);
				qsql_be->error = TRUE;
				PERR (" error on create_event:%s", qsql_be->err);
			}
			else
			{
				((QofInstance *) ent)->dirty = FALSE;
				qsql_be->error = FALSE;
				g_free (qb.sql_str);
				g_free (gstr);
				LEAVE (" ");
				break;
			}
			/* insert sqlite_kvp data */
			slots = qof_instance_get_slots ((QofInstance *) ent);
			if (slots)
			{
				/* id, guid, path, type, value */
				qb.sql_str = qof_sql_entity_insert (ent);
				if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
					NULL, &qb, &qsql_be->err) != SQLITE_OK)
				{
					qof_error_set_be (be, qsql_be->err_insert);
					qsql_be->error = TRUE;
					PERR (" error on KVP create_event:%s", qsql_be->err);
				}
				else
				{
					((QofInstance *) ent)->dirty = FALSE;
					qsql_be->error = FALSE;
					g_free (qb.sql_str);
					g_free (gstr);
					LEAVE (" ");
					break;
				}
			}
			g_free (qb.sql_str);
			g_free (gstr);
			LEAVE (" ");
			break;
		}
	default:
		break;
	}
}

static void
qsql_modify (QofBackend * be, QofInstance * inst)
{
	struct QsqlBuilder qb;
	QSQLiteBackend *qsql_be;

	qsql_be = (QSQLiteBackend *) be;
	qb.qsql_be = qsql_be;
	if (!inst)
		return;
	if (!inst->param)
		return;
	if (loading)
		return;
	if (!inst->param->param_setfcn)
		return;
	ENTER (" modified %s param:%s", ((QofEntity *) inst)->e_type,
		inst->param->param_name);
	qb.sql_str = qof_sql_entity_update ((QofEntity*)inst);
	DEBUG (" sql_str=%s", qb.sql_str);
	if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
			NULL, &qb, &qsql_be->err) != SQLITE_OK)
	{
		qof_error_set_be (be, qsql_be->err_update);
		qsql_be->error = TRUE;
		PERR (" error on modify:%s", qsql_be->err);
	}
	else
	{
		inst->dirty = FALSE;
		g_free (qb.sql_str);
		qsql_be->error = FALSE;
		LEAVE (" ");
		return;
	}
	LEAVE (" ");
}

/** \todo need a KVP version to load data into the slots */
static gint
record_foreach (gpointer builder, gint col_num, gchar ** strings,
	gchar ** columnNames)
{
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder *qb;
	const QofParam *param;
	QofInstance *inst;
	QofEntity *ent;
	gint i;

	g_return_val_if_fail (builder, QSQL_ERROR);
	qb = (struct QsqlBuilder *) builder;
	qsql_be = qb->qsql_be;
	qof_event_suspend ();
	inst = (QofInstance *) qof_object_new_instance (qb->e_type,	qsql_be->book);
	ent = &inst->entity;
	for (i = 0; i < col_num; i++)
	{
		/* get param and set as string */
		param = qof_class_get_parameter (qb->e_type, columnNames[i]);
		if (!param)
			continue;
		/* set the inst->param entry */
		inst->param = param;
		if (0 == safe_strcmp (columnNames[i], QOF_TYPE_GUID))
		{
			GUID *guid;
			guid = guid_malloc ();
			if (!string_to_guid (strings[i], guid))
			{
				DEBUG (" set guid failed:%s", strings[i]);
				return QSQL_ERROR;
			}
			qof_entity_set_guid (ent, guid);
		}
		if (strings[i])
			qof_util_param_set_string (ent, param, strings[i]);
	}
	qof_event_resume ();
	return SQLITE_OK;
}

static void
update_dirty (gpointer value, gpointer builder)
{
	QofInstance *inst;
	QofEntity *ent;
	struct QsqlBuilder *qb;
	QSQLiteBackend *qsql_be;
	QofBackend *be;
	gchar *gstr;

	qb = (struct QsqlBuilder *) builder;
	qsql_be = qb->qsql_be;
	be = (QofBackend *) qsql_be;
	ent = (QofEntity *) value;
	inst = (QofInstance *) ent;
	if (!inst->dirty)
		return;
	ENTER (" ");
	gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
	guid_to_string_buff (qof_entity_get_guid (ent), gstr);
	/* qof_class_param_foreach  */
	qb->sql_str = qof_sql_entity_update (ent);
	DEBUG (" update=%s", qb->sql_str);
	if (sqlite_exec (qsql_be->sqliteh, qb->sql_str,
			NULL, qb, &qsql_be->err) != SQLITE_OK)
	{
		qof_error_set_be (be, qsql_be->err_update);
		qsql_be->error = TRUE;
		PERR (" error on update_dirty:%s", qsql_be->err);
	}
	else
	{
		qof_error_get_message_be (be);
		qsql_be->error = FALSE;
		inst->dirty = FALSE;
	}
	LEAVE (" ");
	g_free (gstr);
	return;
}

static gint
create_dirty_list (gpointer builder, gint col_num, gchar ** strings,
	gchar ** columnNames)
{
	struct QsqlBuilder *qb;
	QofInstance *inst;
	const QofParam *param;
	gchar *value, *columnName, *tmp;

	param = NULL;
	qb = (struct QsqlBuilder *) builder;
	/* qb->ent is the live data, strings is the sqlite data */
	inst = (QofInstance *) qb->ent;
	qb->exists = TRUE;
	if (!inst->dirty)
		return SQLITE_OK;
	columnName = columnNames[col_num];
	tmp = strings[col_num];
	param = qof_class_get_parameter (qb->ent->e_type, columnName);
	if (!param)
		return SQLITE_OK;
	value = qof_util_param_to_string (qb->ent, param);
	qb->dirty = param;
	qb->dirty_list = g_list_prepend (qb->dirty_list, qb->ent);
	DEBUG (" dirty_list=%d", g_list_length (qb->dirty_list));
	return SQLITE_OK;
}

static gint
mark_entity (gpointer builder, gint col_num, gchar ** strings,
	gchar ** columnNames)
{
	struct QsqlBuilder *qb;

	qb = (struct QsqlBuilder *) builder;
	qb->exists = TRUE;
	return SQLITE_OK;
}

static void
qsql_create (QofBackend * be, QofInstance * inst)
{
	gchar *gstr;
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder qb;
	QofEntity *ent;

	qsql_be = (QSQLiteBackend *) be;
	if (!inst)
		return;
	if (loading)
		return;
	ent = (QofEntity *) inst;
	qof_event_suspend ();
	qb.has_slots = FALSE;
	ENTER (" %s", ent->e_type);
	gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
	guid_to_string_buff (qof_entity_get_guid (ent), gstr);
	qb.sql_str =
		g_strdup_printf ("SELECT * FROM %s where guid = \"%s\";",
		ent->e_type, gstr);
	PINFO (" check exists: %s", qb.sql_str);
	qb.ent = ent;
	qb.dirty_list = NULL;
	qb.exists = FALSE;
	if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
			mark_entity, &qb, &qsql_be->err) != SQLITE_OK)
	{
		qof_error_set_be (be, qsql_be->err_update);
		qsql_be->error = TRUE;
		PERR (" error on select :%s", qsql_be->err);
	}
	if (!qb.exists)
	{
		/* create new entity */
		qb.sql_str = qof_sql_entity_insert (ent);
		DEBUG (" sql_str= %s", qb.sql_str);
		if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
				NULL, qsql_be, &qsql_be->err) != SQLITE_OK)
		{
			qof_error_set_be (be, qsql_be->err_insert);
			qsql_be->error = TRUE;
			PERR (" error creating new entity:%s", qsql_be->err);
		}
	}
	g_free (qb.sql_str);
	g_free (gstr);
	qof_event_resume ();
	LEAVE (" ");
}

static void
check_state (QofEntity * ent, gpointer builder)
{
	gchar *gstr;
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder *qb;
	QofBackend *be;
	QofInstance *inst;

	qb = (struct QsqlBuilder *) builder;
	qsql_be = qb->qsql_be;
	be = (QofBackend *) qsql_be;
	inst = (QofInstance *) ent;
	if (!inst->dirty)
		return;
	/* check if this entity already exists */
	gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
	guid_to_string_buff (qof_entity_get_guid (ent), gstr);
	qb->sql_str =
		g_strdup_printf ("SELECT * FROM %s where guid = \"%s\";",
		ent->e_type, gstr);
	qb->ent = ent;
	qb->dirty_list = NULL;
	/* assume entity does not yet exist in backend,
	   e.g. being copied from another session. */
	qb->exists = FALSE;
	qb->qsql_be = qsql_be;
	/* update each dirty instance */
	/* Make a GList of dirty instances
	   Don't update during a SELECT,
	   UPDATE will fail with DB_LOCKED */
	if (sqlite_exec (qsql_be->sqliteh, qb->sql_str,
			create_dirty_list, qb, &qsql_be->err) != SQLITE_OK)
	{
		qof_error_set_be (be, qsql_be->err_update);
		qsql_be->error = TRUE;
		PERR (" error on check_state:%s", qsql_be->err);
	}
	if (!qb->exists)
	{
		/* create new entity */
		qb->sql_str = qof_sql_entity_insert (ent);
		DEBUG (" sql_str= %s", qb->sql_str);
		if (sqlite_exec (qsql_be->sqliteh, qb->sql_str,
				NULL, qb, &qsql_be->err) != SQLITE_OK)
		{
			qof_error_set_be (be, qsql_be->err_insert);
			qsql_be->error = TRUE;
			PERR (" error on check_state create_new:%s", qsql_be->err);
		}
		g_free (qb->sql_str);
	}
	/* update instead */
	g_list_foreach (qb->dirty_list, update_dirty, &qb);
	g_free (qb->sql_str);
	g_free (gstr);
}

/** \brief chekc kvp data once per record

creates a new KvpFrame as data for a GHashTable with the guid as key

 \todo improve error checking support in case the SQLite data is
tweaked manually.

*/
static gint
build_kvp_table (gpointer builder, gint col_num, gchar ** strings,
	gchar ** columnNames)
{
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder *qb;
	KvpFrame *frame;
	KvpValueType type;
	KvpValue *value;
	gulong max;
	gchar *tail;

	g_return_val_if_fail (builder, QSQL_ERROR);
	qb = (struct QsqlBuilder *) builder;
	max = 0;
	qsql_be = qb->qsql_be;
	g_return_val_if_fail ((col_num < 4), QSQL_ERROR);
	g_return_val_if_fail (strings[2], QSQL_ERROR);
	frame = kvp_frame_new ();
	/* columnNames = fields strings = values
	   [0]=kvp_id, [1]=guid, [2]=path, [3]=type, [4]=value
	   get type from type_string */
	type = qof_id_to_kvp_value_type (strings[3]);
	if (type == 0)
	{
		PERR (" invalid type returned from kvp table");
		return QSQL_ERROR;
	}
	/* use the type to make a KvpValue from value */
	value = string_to_kvp_value (strings[4], type);
	if (!value)
	{
		PERR (" invalid KvpValue for type: %d", type);
		return QSQL_ERROR;
	}
	/* add the KvpValue to the frame at path */
	kvp_frame_set_value (frame, strings[2], value);
	/* index the frame under the entity GUID */
	g_hash_table_insert (qsql_be->kvp_table, strings[1], frame);
	/* index the guid under the kvp_id */
	g_hash_table_insert (qsql_be->kvp_id, strings[0], strings[1]);
	errno = 0;
	max = strtol (strings[0], &tail, 0);
	if (errno == 0)
	{
		qsql_be->index = (max > qsql_be->index) ? max : qsql_be->index;
	}
	return SQLITE_OK;
}

/** only call once per book */
static void
qsql_load_kvp (QSQLiteBackend * qsql_be)
{
	struct QsqlBuilder qb;
	QofBackend *be;
	gint sq_code;

	g_return_if_fail (qsql_be);
	sq_code = SQLITE_OK;
	be = (QofBackend *) qsql_be;
	qb.sql_str =
		g_strdup_printf ("SELECT kvp_id from %s;", QSQL_KVP_TABLE);
	sq_code = sqlite_exec (qsql_be->sqliteh, qb.sql_str, build_kvp_table,
			&qb, &qsql_be->err);
	/* catch older files without a sqlite_kvp table */
	if (sq_code == SQLITE_ERROR)
	{
		g_free (qb.sql_str);
		qb.sql_str =
			g_strdup_printf ("CREATE TABLE %s (%s, %s, %s, %s, %s, %s",
			QSQL_KVP_TABLE, "kvp_id int primary key not null",
			"guid char(32)", "path mediumtext", "type mediumtext",
			"value text", END_DB_VERSION);
		PINFO (" creating kvp table. sql=%s", qb.sql_str);
		if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
			record_foreach, &qb, &qsql_be->err) != SQLITE_OK)
		{
			qsql_be->error = TRUE;
			PERR (" unable to create kvp table:%s", qsql_be->err);
		}
	}
	else if (sq_code != SQLITE_OK)
	{
		qof_error_set_be (be, qsql_be->err_create);
		qsql_be->error = TRUE;
		PERR (" error on KVP select:%s:%s:%d", qb.sql_str, qsql_be->err, sq_code);
	}
	g_free (qb.sql_str);
}

/** receives QSQLiteBackend from QofBackend */
static void
qsql_class_foreach (QofObject * obj, gpointer data)
{
	struct QsqlBuilder qb;
	QSQLiteBackend *qsql_be;
	QofBackend *be;

	qsql_be = (QSQLiteBackend *) data;
	be = (QofBackend *) qsql_be;
	qb.qsql_be = qsql_be;
	qb.e_type = obj->e_type;
	ENTER (" obj_type=%s", qb.e_type);
	switch (qsql_be->stm_type)
	{
	case SQL_NONE:
	case SQL_INSERT:
	case SQL_DELETE:
	case SQL_UPDATE:
		{
			break;
		}
	case SQL_CREATE:
		{
			/* KVP is handled separately */
			qb.sql_str = qof_sql_object_create_table (obj);
			if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
					NULL, NULL, &qsql_be->err) != SQLITE_OK)
			{
				qof_error_set_be (be, qsql_be->err_create);
				qsql_be->error = TRUE;
				PERR (" error on SQL_CREATE:%s", qsql_be->err);
			}
			g_free (qb.sql_str);
			break;
		}
	case SQL_LOAD:
		{
			qb.sql_str =
				g_strdup_printf ("SELECT * FROM %s;", obj->e_type);
			PINFO (" sql=%s", qb.sql_str);
			if (sqlite_exec (qsql_be->sqliteh, qb.sql_str,
					record_foreach, &qb, &qsql_be->err) != SQLITE_OK)
			{
				qsql_be->error = TRUE;
				PERR (" error on SQL_LOAD:%s", qsql_be->err);
			}
			break;
		}
	case SQL_WRITE:
		{
			if (!qof_book_not_saved (qsql_be->book))
				break;
			qof_object_foreach (obj->e_type, qsql_be->book, check_state,
				&qb);
			break;
		}
	}
	LEAVE (" ");
}

static void
qsql_backend_createdb (QofBackend * be, QofSession * session)
{
	FILE *f;
	QSQLiteBackend *qsql_be;
	struct QsqlBuilder qb;

	g_return_if_fail (be || session);
	ENTER (" ");
	qsql_be = (QSQLiteBackend *) be;
	qsql_be->stm_type = SQL_CREATE;
	qb.qsql_be = qsql_be;
	qsql_be->book = qof_session_get_book (session);
	DEBUG (" create_file %s", qsql_be->fullpath);
	f = fopen (qsql_be->fullpath, "a+");
	if (f)
		fclose (f);
	else
	{
		qof_error_set (session, qof_error_register
			(_("Unable to open the output file '%s' - do you have "
					"permission to create this file?"), TRUE));
		qsql_be->error = TRUE;
		LEAVE (" unable to create new file '%s'", qsql_be->fullpath);
		return;
	}
	qsql_be->sqliteh =
		sqlite_open (qsql_be->fullpath, 0644, &qsql_be->err);
	if (!qsql_be->sqliteh)
	{
		qof_error_set_be (be, qsql_be->err_create);
		qsql_be->error = TRUE;
		LEAVE (" unable to open sqlite:%s", qsql_be->err);
		return;
	}
	qof_object_foreach_type (qsql_class_foreach, qsql_be);
	LEAVE (" ");
}

static void
qsql_backend_opendb (QofBackend * be, QofSession * session)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail (be || session);
	ENTER (" ");
	qsql_be = (QSQLiteBackend *) be;
	qsql_be->sqliteh =
		sqlite_open (qsql_be->fullpath, 0666, &qsql_be->err);
	if (!qsql_be->sqliteh)
	{
		qof_error_set_be (be, qof_error_register
			(_("Unable to open the sqlite database '%s'."), TRUE));
		qsql_be->error = TRUE;
		PERR (" %s", qsql_be->err);
	}
	LEAVE (" %s", qsql_be->fullpath);
}

static void
qsqlite_session_begin (QofBackend * be, QofSession * session,
	const gchar * book_path, gboolean ignore_lock,
	gboolean create_if_nonexistent)
{
	QSQLiteBackend *qsql_be;
	gchar **pp;
	struct stat statinfo;
	gint stat_val;

	g_return_if_fail (be);
	ENTER (" book_path=%s", book_path);
	qsql_be = (QSQLiteBackend *) be;
	qsql_be->fullpath = NULL;
	if (book_path == NULL)
	{
		qof_error_set_be (be, qof_error_register
			(_("Please provide a filename for sqlite."), FALSE));
		qsql_be->error = TRUE;
		LEAVE (" bad URL");
		return;
	}
	/* book_path => sqlite_file_name */
	pp = g_strsplit (book_path, ":", 2);
	if (0 == safe_strcmp (pp[0], ACCESS_METHOD))
	{
		qsql_be->fullpath = g_strdup (pp[1]);
		g_strfreev (pp);
	}
	else
		qsql_be->fullpath = g_strdup (book_path);
	be->fullpath = g_strdup (qsql_be->fullpath);
	PINFO (" final path = %s", qsql_be->fullpath);
	stat_val = g_stat (qsql_be->fullpath, &statinfo);
	if (!S_ISREG (statinfo.st_mode) || statinfo.st_size == 0)
		qsql_backend_createdb (be, session);
	if (!qsql_be->error)
		qsql_backend_opendb (be, session);
	if (qof_error_check_be (be) || qsql_be->error)
	{
		LEAVE (" open failed");
		return;
	}
	qsql_be->create_handler =
		qof_event_register_handler (create_event, qsql_be);
	qsql_be->delete_handler =
		qof_event_register_handler (delete_event, qsql_be);
	LEAVE (" db=%s", qsql_be->fullpath);
}

static void
qsqlite_db_load (QofBackend * be, QofBook * book)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail (be);
	ENTER (" ");
	loading = TRUE;
	qsql_be = (QSQLiteBackend *) be;
	qsql_be->stm_type = SQL_LOAD;
	qsql_be->book = book;
	/* iterate over registered objects */
	qof_object_foreach_type (qsql_class_foreach, qsql_be);
	qsql_load_kvp (qsql_be);
	loading = FALSE;
	LEAVE (" ");
}

static void
qsqlite_write_db (QofBackend * be, QofBook * book)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail (be);
	qsql_be = (QSQLiteBackend *) be;
	qsql_be->stm_type = SQL_WRITE;
	qsql_be->book = book;
	/* update each record with current state */
	qof_object_foreach_type (qsql_class_foreach, qsql_be);
}

static gboolean
qsql_determine_file_type (const gchar * path)
{
	if (!path)
		return FALSE;
	return TRUE;
}

static void
qsqlite_session_end (QofBackend * be)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail (be);
	qsql_be = (QSQLiteBackend *) be;
	if (qsql_be->sqliteh)
		sqlite_close (qsql_be->sqliteh);
}

static void
qsqlite_destroy_backend (QofBackend * be)
{
	QSQLiteBackend *qsql_be;

	g_return_if_fail (be);
	qsql_be = (QSQLiteBackend *) be;
	g_hash_table_destroy (qsql_be->kvp_table);
	g_hash_table_destroy (qsql_be->kvp_id);
	qof_event_unregister_handler (qsql_be->create_handler);
	qof_event_unregister_handler (qsql_be->delete_handler);
	g_free (be);
	g_free (qsql_be);
}

static void
qsql_provider_free (QofBackendProvider * prov)
{
	prov->provider_name = NULL;
	prov->access_method = NULL;
	g_free (prov);
}

/** \brief Starts the backend and creates the context

 \note Take care when handling the main QSQLiteBackend
context and the QsqlBuilder context. QSQLiteBackend contains the
long-term data, QsqlBuilder the transient. Only QSQLiteBackend
is guaranteed to exist at any one time. All functions need to be
able to locate the QSQLiteBackend. Functions started from the
QofBackend routines or from the event handlers will be passed the
QofBackend which can be cast to QSQLiteBackend. Internal functions
create a local QsqlBuilder struct and set the QSQLiteBackend pointer
before passing a pointer to the QsqlBuilder. Use the qsql_ prefix
only for functions that are started from QofBackend
and the _event suffix for QofEvent.

*/
static QofBackend *
qsql_backend_new (void)
{
	QSQLiteBackend *qsql_be;
	QofBackend *be;

	ENTER (" ");
	qsql_be = g_new0 (QSQLiteBackend, 1);
	be = (QofBackend *) qsql_be;
	qof_backend_init (be);
	qsql_be->kvp_table = g_hash_table_new (g_str_hash, g_str_equal);
	qsql_be->kvp_id = g_hash_table_new (g_str_hash, g_str_equal);
	qsql_be->dbversion = QOF_OBJECT_VERSION;
	qsql_be->stm_type = SQL_NONE;
	qsql_be->err_delete =
		qof_error_register (_("Unable to delete record."), FALSE);
	qsql_be->err_create =
		qof_error_register (_("Unable to create record."), FALSE);
	qsql_be->err_insert =
		qof_error_register (_("Unable to insert a new record."), FALSE);
	qsql_be->err_update =
		qof_error_register (_("Unable to update existing record."), FALSE);
	be->session_begin = qsqlite_session_begin;

	be->session_end = qsqlite_session_end;
	be->destroy_backend = qsqlite_destroy_backend;
	be->load = qsqlite_db_load;
	be->save_may_clobber_data = NULL;
	/* begin: create an empty entity if none exists,
	   even if events are suspended. */
	be->begin = qsql_create;
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

void
qof_sqlite_provider_init (void)
{
	QofBackendProvider *prov;

	ENTER (" ");
	bindtextdomain (PACKAGE, LOCALE_DIR);
	qof_sql_entity_set_kvp_tablename (QSQL_KVP_TABLE);
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "QOF SQLite Backend Version 0.4";
	prov->access_method = ACCESS_METHOD;
	prov->partial_book_supported = TRUE;
	prov->backend_new = qsql_backend_new;
	prov->check_data_type = qsql_determine_file_type;
	prov->provider_free = qsql_provider_free;
	qof_backend_register_provider (prov);
	LEAVE (" ");
}

/* ================= END OF FILE =================== */
