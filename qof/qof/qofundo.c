/***************************************************************************
 *            qofundo.c
 *
 *  Thu Aug 25 09:19:17 2005
 *  Copyright  2005,2006  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************************/
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA  02110-1301,  USA
 */

#include "config.h"
#include <glib.h>
#include <qof.h>
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <errno.h>
#include "qofbook-p.h"
#include "qofundo-p.h"
#include "qofundo.h"

static QofLogModule log_module = QOF_MOD_UNDO;

typedef enum
{
	UNDO_NOOP = 0,
	UNDO_CREATE,
	UNDO_DELETE,
	UNDO_MODIFY
} QofUndoAction;

struct QofUndoEntity_t
{
	const QofParam *param;		/* static anyway so only store a pointer */
	const GUID *guid;			/* enable re-creation of this entity */
	QofIdType type;				/* ditto param, static. */
	gchar *value;				/* cached string? */
	gchar *path;				/* for KVP */
	QofIdType choice;			/* For QOF_TYPE_CHOICE */
	QofUndoAction how;			/* how to act on the undo */
};

struct QofUndoOperation_t
{
	const gchar *label;
	QofTime *qt;
	GList *entity_list;			/* GList of qof_undo_entity* */
};

static void
set_param (QofEntity * ent, const QofParam * param, 
					  gchar * value)
{
	gchar *tail;
	QofNumeric cli_numeric;
	gboolean cli_bool;
	gint32 cli_i32;
	gint64 cli_i64;
	QofTime *cli_time;
	GUID *cm_guid;
	void (*string_setter) (QofEntity *, gchar *);
	void (*time_setter) (QofEntity *, QofTime *);
	void (*i32_setter) (QofEntity *, gint32);
	void (*i64_setter) (QofEntity *, gint64);
	void (*numeric_setter) (QofEntity *, QofNumeric);
	void (*boolean_setter) (QofEntity *, gboolean);
	void (*guid_setter) (QofEntity *, const GUID *);

	if (0 == safe_strcmp (param->param_type, QOF_TYPE_STRING))
	{
		string_setter =
			(void (*)(QofEntity *, gchar *)) param->param_setfcn;
		if (string_setter)
		{
			param->param_setfcn (ent, value);
		}
	}
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_GUID))
	{
		cm_guid = g_new (GUID, 1);
		if (TRUE == string_to_guid (value, cm_guid))
		{
			guid_setter =
				(void (*)(QofEntity *, const GUID *)) param->param_setfcn;
			if (guid_setter != NULL)
			{
				guid_setter (ent, cm_guid);
			}
		}
	}
	if ((0 == safe_strcmp (param->param_type, QOF_TYPE_NUMERIC)) ||
		(safe_strcmp (param->param_type, QOF_TYPE_DEBCRED) == 0))
	{
		numeric_setter =
			(void (*)(QofEntity *, QofNumeric)) param->param_setfcn;
		qof_numeric_from_string (value, &cli_numeric);
		if (numeric_setter != NULL)
		{
			numeric_setter (ent, cli_numeric);
		}
	}
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_BOOLEAN))
	{
		cli_bool = FALSE;
		if (qof_util_bool_to_int (value) == 1)
		{
			cli_bool = TRUE;
		}
		boolean_setter =
			(void (*)(QofEntity *, gboolean)) param->param_setfcn;
		if (boolean_setter != NULL)
		{
			boolean_setter (ent, cli_bool);
		}
	}
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_INT32))
	{
		errno = 0;
		cli_i32 = (gint32) strtol (value, &tail, 0);
		if (errno == 0)
		{
			i32_setter =
				(void (*)(QofEntity *, gint32)) param->param_setfcn;
			if (i32_setter != NULL)
			{
				i32_setter (ent, cli_i32);
			}
		}
		else
		{
			PERR (" Cannot convert %s into a number: "
				"an overflow has been detected.", value);
		}
	}
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_INT64))
	{
		errno = 0;
		cli_i64 = (gint64) strtol (value, &tail, 0);
		if (errno == 0)
		{
			i64_setter =
				(void (*)(QofEntity *, gint64)) param->param_setfcn;
			if (i64_setter != NULL)
			{
				i64_setter (ent, cli_i64);
			}
		}
		else
		{
			PERR (" Cannot convert %s into a number: "
				"an overflow has been detected.", value);
		}
	}
	if (0 ==safe_strcmp (param->param_type, QOF_TYPE_TIME))
	{
		QofDate *qd;

		qd = qof_date_parse (value, QOF_DATE_FORMAT_UTC);
		cli_time = qof_date_to_qtime (qd);
		time_setter = 
			(void (*)(QofEntity *, QofTime *)) param->param_setfcn;
		if ((time_setter != NULL) && qof_time_is_valid (cli_time))
		{
			time_setter (ent, cli_time);
		}
	}
#ifndef QOF_DISABLE_DEPRECATED
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_DATE))
	{
		Timespec cli_date;
		time_t cli_time_t;
		void (*date_setter) (QofEntity *, Timespec);
		struct tm cli_time;

		date_setter =
			(void (*)(QofEntity *, Timespec)) param->param_setfcn;
		strptime (value, QOF_UTC_DATE_FORMAT, &cli_time);
		cli_time_t = mktime (&cli_time);
		timespecFromTime_t (&cli_date, cli_time_t);
		if (date_setter != NULL)
		{
			date_setter (ent, cli_date);
		}
	}
#endif
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_CHAR))
	{
		param->param_setfcn (ent, value);
	}
}

void
qof_undo_set_param (QofEntity * ent, const QofParam * param, 
					  gchar * value)
{
	qof_undo_modify ((QofInstance*)ent, param);
	set_param (ent, param, value);
	qof_undo_commit ((QofInstance*)ent, param);
}

static void
undo_from_kvp_helper (const gchar * path, KvpValue * content,
	gpointer data)
{
	QofUndoEntity *undo_entity;

	undo_entity = (QofUndoEntity *) data;
	undo_entity->path = g_strdup (path);
	undo_entity->value = kvp_value_to_bare_string (content);
}

QofUndoEntity *
qof_prepare_undo (QofEntity * ent, const QofParam * param)
{
	QofUndoEntity *undo_entity;
	KvpFrame *undo_frame;

	undo_frame = NULL;
	undo_entity = g_new0 (QofUndoEntity, 1);
	undo_entity->guid = qof_entity_get_guid (ent);
	undo_entity->param = param;
	undo_entity->how = UNDO_MODIFY;
	undo_entity->type = ent->e_type;
	undo_entity->value = 
		qof_book_merge_param_as_string ((QofParam*) param, ent);
	if (0 == (safe_strcmp (param->param_type, QOF_TYPE_KVP)))
	{
		undo_frame = kvp_frame_copy (param->param_getfcn (ent, param));
		kvp_frame_for_each_slot (undo_frame, undo_from_kvp_helper,
			undo_entity);
	}
	/* need to do COLLECT and CHOICE */
	return undo_entity;
}

static void
qof_reinstate_entity (QofUndoEntity * undo_entity, QofBook * book)
{
	const QofParam *undo_param;
	QofCollection *coll;
	QofEntity *ent;

	undo_param = undo_entity->param;
	if (!undo_param)
		return;
	PINFO (" reinstate:%s", undo_entity->type);
	coll = qof_book_get_collection (book, undo_entity->type);
	if (!coll)
		return;
	ent = qof_collection_lookup_entity (coll, undo_entity->guid);
	if (!ent)
		return;
	PINFO (" undoing %s %s", undo_param->param_name, undo_entity->value);
	set_param (ent, undo_param, undo_entity->value);
}

static void
qof_recreate_entity (QofUndoEntity * undo_entity, QofBook * book)
{
	QofEntity *ent;
	const GUID *guid;
	QofIdType type;
	QofInstance *inst;

	guid = undo_entity->guid;
	type = undo_entity->type;
	g_return_if_fail (guid || type);
	inst = (QofInstance *) qof_object_new_instance (type, book);
	ent = (QofEntity *) inst;
	qof_entity_set_guid (ent, guid);
}

static void
qof_dump_entity (QofUndoEntity * undo_entity, QofBook * book)
{
	QofCollection *coll;
	QofEntity *ent;
	const GUID *guid;
	QofIdType type;

	type = undo_entity->type;
	guid = undo_entity->guid;
	g_return_if_fail (type || book);
	coll = qof_book_get_collection (book, type);
	ent = qof_collection_lookup_entity (coll, guid);
	qof_entity_release (ent);
}

void
qof_book_undo (QofBook * book)
{
	QofUndoOperation *undo_operation;
	QofUndoEntity *undo_entity;
	QofUndo *book_undo;
	GList *ent_list;
	gint length;

	book_undo = book->undo_data;
	length = g_list_length (book_undo->undo_list);
	if (book_undo->index_position > 1)
		book_undo->index_position--;
	else
		book_undo->index_position = 0;
	undo_operation =
		(QofUndoOperation
		*) (g_list_nth (book_undo->undo_list,
			book_undo->index_position))->data;
	g_return_if_fail (undo_operation);
	ent_list = undo_operation->entity_list;
	while (ent_list != NULL)
	{
		undo_entity = (QofUndoEntity *) ent_list->data;
		if (!undo_entity)
			break;
		switch (undo_entity->how)
		{
		case UNDO_MODIFY:
			{
				qof_reinstate_entity (undo_entity, book);
				break;
			}
		case UNDO_CREATE:
			{
				qof_recreate_entity (undo_entity, book);
				break;
			}
		case UNDO_DELETE:
			{
				qof_dump_entity (undo_entity, book);
				break;
			}
		case UNDO_NOOP:
			{
				break;
			}
		}
		ent_list = g_list_next (ent_list);
	}
}

void
qof_book_redo (QofBook * book)
{
	QofUndoOperation *undo_operation;
	QofUndoEntity *undo_entity;
	QofUndo *book_undo;
	GList *ent_list;
	gint length;

	book_undo = book->undo_data;
	undo_operation =
		(QofUndoOperation
		*) (g_list_nth (book_undo->undo_list,
			book_undo->index_position))->data;
	if (!undo_operation)
		return;
	ent_list = undo_operation->entity_list;
	while (ent_list != NULL)
	{
		undo_entity = (QofUndoEntity *) ent_list->data;
		if (!undo_entity)
			break;
		switch (undo_entity->how)
		{
		case UNDO_MODIFY:
			{
				qof_reinstate_entity (undo_entity, book);
				break;
			}
		case UNDO_CREATE:
			{
				qof_dump_entity (undo_entity, book);
				break;
			}
		case UNDO_DELETE:
			{
				qof_recreate_entity (undo_entity, book);
				break;
			}
		case UNDO_NOOP:
			{
				break;
			}
		}
		ent_list = g_list_next (ent_list);
	}
	length = g_list_length (book_undo->undo_list);
	if (book_undo->index_position < length)
		book_undo->index_position++;
	else
		book_undo->index_position = length;
}

void
qof_book_clear_undo (QofBook * book)
{
	QofUndoOperation *operation;
	QofUndo *book_undo;

	if (!book)
		return;
	book_undo = book->undo_data;
	while (book_undo != NULL)
	{
		operation = (QofUndoOperation *) book_undo->undo_list->data;
		if(operation->entity_list)
			g_list_free (operation->entity_list);
		book_undo->undo_list = g_list_next (book_undo->undo_list);
	}
	book_undo->index_position = 0;
	g_free (book_undo->undo_label);
}

gboolean
qof_book_can_undo (QofBook * book)
{
	QofUndo *book_undo;
	gint length;

	book_undo = book->undo_data;
	length = g_list_length (book_undo->undo_list);
	if ((book_undo->index_position == 0) || (length == 0))
		return FALSE;
	return TRUE;
}

gboolean
qof_book_can_redo (QofBook * book)
{
	QofUndo *book_undo;
	gint length;

	book_undo = book->undo_data;
	length = g_list_length (book_undo->undo_list);
	if ((book_undo->index_position == length) || (length == 0))
		return FALSE;
	return TRUE;
}

QofUndoOperation *
qof_undo_new_operation (QofBook * book, gchar * label)
{
	QofUndoOperation *undo_operation;
	QofUndo *book_undo;

	undo_operation = NULL;
	book_undo = book->undo_data;
	undo_operation = g_new0 (QofUndoOperation, 1);
	undo_operation->label = label;
	undo_operation->qt = qof_time_get_current();
	undo_operation->entity_list = NULL;
	g_list_foreach (book_undo->undo_cache,
		qof_undo_new_entry, undo_operation);
	return undo_operation;
}

void
qof_undo_new_entry (gpointer cache, gpointer operation)
{
	QofUndoOperation *undo_operation;
	QofUndoEntity *undo_entity;

	g_return_if_fail (operation || cache);
	undo_operation = (QofUndoOperation *) operation;
	undo_entity = (QofUndoEntity *) cache;
	g_return_if_fail (undo_operation || undo_entity);
	undo_operation->entity_list =
		g_list_prepend (undo_operation->entity_list, undo_entity);
}

void
qof_undo_create (QofInstance * instance)
{
	QofUndoEntity *undo_entity;
	QofBook *book;
	QofUndo *book_undo;

	if (!instance)
		return;
	book = instance->book;
	book_undo = book->undo_data;
	undo_entity = g_new0 (QofUndoEntity, 1);
	// to undo a create, use a delete.
	undo_entity->how = UNDO_DELETE;
	undo_entity->guid = qof_instance_get_guid (instance);
	undo_entity->type = instance->entity.e_type;
	book_undo->undo_cache =
		g_list_prepend (book_undo->undo_cache, undo_entity);
}

static void
undo_get_entity (QofParam * param, gpointer data)
{
	QofBook *book;
	QofUndo *book_undo;
	QofInstance *instance;
	QofUndoEntity *undo_entity;

	instance = (QofInstance *) data;
	book = instance->book;
	book_undo = book->undo_data;
	g_return_if_fail (instance || param);
	undo_entity = qof_prepare_undo (&instance->entity, param);
	book_undo->undo_cache =
		g_list_prepend (book_undo->undo_cache, undo_entity);
}

void
qof_undo_delete (QofInstance * instance)
{
	QofUndoEntity *undo_entity;
	QofIdType type;
	QofUndo *book_undo;
	QofBook *book;

	if (!instance)
		return;
	book = instance->book;
	book_undo = book->undo_data;
	// now need to store each parameter in a second entity, MODIFY.
	type = instance->entity.e_type;
	qof_class_param_foreach (type, undo_get_entity, instance);
	undo_entity = g_new0 (QofUndoEntity, 1);
	// to undo a delete, use a create.
	undo_entity->how = UNDO_CREATE;
	undo_entity->guid = qof_instance_get_guid (instance);
	undo_entity->type = type;
	book_undo->undo_cache =
		g_list_prepend (book_undo->undo_cache, undo_entity);
}

void
qof_undo_modify (QofInstance * instance, const QofParam * param)
{
	QofBook *book;
	QofUndo *book_undo;
	QofUndoEntity *undo_entity;

	if (!instance || !param)
		return;
	book = instance->book;
	book_undo = book->undo_data;
	// handle if record is called without a commit.
	undo_entity = qof_prepare_undo (&instance->entity, param);
	book_undo->undo_cache =
		g_list_prepend (book_undo->undo_cache, undo_entity);
	// set the initial state that undo will reinstate.
	if (book_undo->index_position == 0)
	{
		book_undo->undo_list = g_list_prepend (book_undo->undo_list,
			qof_undo_new_operation (book, "initial"));
		book_undo->index_position++;
	}
}

void
qof_undo_commit (QofInstance * instance, const QofParam * param)
{
	QofUndoEntity *undo_entity;
	QofUndo *book_undo;
	QofBook *book;

	if (!instance || !param)
		return;
	book = instance->book;
	book_undo = book->undo_data;
	undo_entity = qof_prepare_undo (&instance->entity, param);
	book_undo->undo_cache =
		g_list_prepend (book_undo->undo_cache, undo_entity);
}

void
qof_book_start_operation (QofBook * book, gchar * label)
{
	QofUndo *book_undo;

	book_undo = book->undo_data;
	if (book_undo->undo_operation_open && book_undo->undo_cache)
	{
		g_list_free (book_undo->undo_cache);
		book_undo->undo_operation_open = FALSE;
		if (book_undo->undo_label)
			g_free (book_undo->undo_label);
	}
	book_undo->undo_label = g_strdup (label);
	book_undo->undo_operation_open = TRUE;
}

void
qof_book_end_operation (QofBook * book)
{
	QofUndo *book_undo;

	book_undo = book->undo_data;
	book_undo->undo_list = g_list_prepend (book_undo->undo_list,
		qof_undo_new_operation (book, book_undo->undo_label));
	book_undo->index_position++;
	g_list_free (book_undo->undo_cache);
	book_undo->undo_operation_open = FALSE;
}

QofTime *
qof_book_undo_first_modified (QofBook * book)
{
	QofUndoOperation *undo_operation;
	QofUndo *book_undo;

	book_undo = book->undo_data;
	undo_operation =
		(QofUndoOperation *) g_list_last (book_undo->undo_list);
	return undo_operation->qt;
}

gint
qof_book_undo_count (QofBook * book)
{
	QofUndo *book_undo;

	book_undo = book->undo_data;
	return g_list_length (book_undo->undo_list);
}

/* ====================== END OF FILE ======================== */
