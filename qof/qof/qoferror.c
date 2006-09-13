/*****************************************************************
 *            qoferror.c
 *
 *  Sun Sep 10 19:55:08 2006
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

#include "qof.h"
#include "qoferror-p.h"
#include "qofsession-p.h"

struct QofError_s
{
	QofErrorId id;
	gchar * message;
	const gchar * filename;
	gboolean use_file;
	QofTime * qt;
};

/* All registered errors - hashed only once per session. */
static GHashTable * error_table = NULL;
static gint32 count = 0;
static QofLogModule log_module = QOF_MOD_ERROR;

void
qof_error_init (void)
{
	error_table = g_hash_table_new (g_direct_hash, g_direct_equal);
#ifndef QOF_DISABLE_DEPRECATED
	set_deprecated_errors ();
#endif
}

static void
qof_error_free (QofError * error)
{
	if (error->qt)
		qof_time_free (error->qt);
	g_free (error->message);
	g_free (error);
}

/* clear the table of registered error values */
static void
clear_table (gpointer key, gpointer value, gpointer user_data)
{
	qof_error_free ((QofError*)value);
}

void
qof_error_close (void)
{
	g_hash_table_foreach (error_table, clear_table, NULL);
	g_hash_table_destroy (error_table);
}

#ifndef QOF_DISABLE_DEPRECATED
void
deprecated_support (QofErrorId id, const gchar * err_message)
{
	QofError * err;

	if (id >= ERR_LAST)
		return;
	err = g_new0 (QofError, 1);
	err->id = id;
	err->message = g_strdup (err_message);
	g_hash_table_insert (error_table, GINT_TO_POINTER(id), err);
}
#endif

QofErrorId
qof_error_register (const gchar * err_message, gboolean use_file)
{
	QofError * err;

	ENTER (" ");
	err = g_new0 (QofError, 1);
	count++;
#ifndef QOF_DISABLE_DEPRECATED
	count += ERR_LAST;
#endif
	err->id = count;
	if (use_file)
	{
		gchar * spec;

		spec = g_strrstr (err_message, "%s");
		use_file = (spec) ? TRUE : FALSE;
	}
	err->use_file = use_file;
	err->message = g_strdup (err_message);
	g_hash_table_insert (error_table, GINT_TO_POINTER(err->id), err);
	LEAVE (" ");
	return err->id;
}

void
qof_error_unregister (QofErrorId id)
{
	QofError * err;
	gboolean result;

	ENTER (" ");
	err = g_hash_table_lookup (error_table, GINT_TO_POINTER(id));
	qof_error_free (err);
	result = g_hash_table_remove (error_table, 
		GINT_TO_POINTER(id));
	if (!result)
		LEAVE ("unable to remove registered error.");
	LEAVE (" ok.");
}

void
qof_error_set (QofSession * session, QofErrorId error)
{
	QofError * err, * set;

	g_return_if_fail (session);
	if (error == QOF_SUCCESS)
		return;
	err = g_hash_table_lookup (error_table, GINT_TO_POINTER(error));
	if (!err)
		return;
	/* create a new error for the list */
	set = g_new0 (QofError, 1);
	if (err->use_file)
		set->message = g_strdup_printf (err->message,
			qof_session_get_file_path (session));
	else
		set->message = g_strdup (err->message);
	set->id = error;
	set->qt = qof_time_get_current ();
	session->backend->error_stack = 
		g_list_prepend (session->backend->error_stack, set);
#ifndef QOF_DISABLE_DEPRECATED
	if (session->error_message)
		g_free (session->error_message);
	session->error_message = g_strdup (set->message);
	session->last_err = error;
	session->backend->last_err = error;
#endif
}

void
qof_error_set_be (QofBackend * be, QofErrorId error)
{
	QofError * err, * set;

	g_return_if_fail (be);
	if (error == QOF_SUCCESS)
		return;
	err = g_hash_table_lookup (error_table, GINT_TO_POINTER(error));
	if (!err)
		return;
	/* create a new error for the list */
	set = g_new0 (QofError, 1);
	set->message = g_strdup (err->message);
	set->id = error;
	set->qt = qof_time_get_current ();
	be->error_stack = g_list_prepend (be->error_stack,
		set);
#ifndef QOF_DISABLE_DEPRECATED
	be->last_err = error;
#endif
}

/* clear the list of actual errors */
static void
clear_list (gpointer value, gpointer user_data)
{
	qof_error_free ((QofError*)value);
}

void
qof_error_clear (QofSession * session)
{
	g_return_if_fail (session);
	if (!session->backend)
		return;
	g_list_foreach (session->backend->error_stack, clear_list, NULL);
	g_list_free (session->backend->error_stack);
	session->backend->error_stack = NULL;
#ifndef QOF_DISABLE_DEPRECATED
	if (session->error_message)
		g_free (session->error_message);
	session->error_message = NULL;
	session->last_err = QOF_SUCCESS;
	session->backend->last_err = QOF_SUCCESS;
#endif
}

QofErrorId
qof_error_check (QofSession * session)
{
	g_return_val_if_fail (session, QOF_FATAL);
	return qof_error_check_be (session->backend);
}

QofErrorId
qof_error_check_be (QofBackend * be)
{
	QofError * err;
	GList * first;

	g_return_val_if_fail (be, QOF_FATAL);
	if (g_list_length (be->error_stack) == 0)
		return QOF_SUCCESS;
	first = g_list_first (be->error_stack);
	err = (QofError*)first->data;
	if (!err)
		return QOF_FATAL;
	return err->id;
}

QofTime *
qof_error_get_time_be (QofBackend * be)
{
	QofError * err;
	GList * first;

	if (g_list_length(be->error_stack) == 0)
		return NULL;
	first = g_list_first (be->error_stack);
	err = (QofError*)first->data;
	return err->qt;
}

QofTime *
qof_error_get_time (QofSession * session)
{
	return qof_error_get_time_be (session->backend);
}

#ifndef QOF_DISABLE_DEPRECATED
static void
set_previous_error (QofBackend * be)
{
	QofError * err;
	GList * pop;

	if (!be)
		return;
	if (g_list_length(be->error_stack) == 0)
		return;
	pop = g_list_last (be->error_stack);
	err = (QofError*)pop->data;
	be->last_err = err->id;
	be->error_msg = err->message;
}
#endif

QofErrorId
qof_error_get_id (QofSession * session)
{
	QofErrorId id;

	g_return_val_if_fail (session, QOF_FATAL);
	id = qof_error_get_id_be (session->backend);
#ifndef QOF_DISABLE_DEPRECATED
	{
		QofError * err;

		err = g_hash_table_lookup (error_table, 
			GINT_TO_POINTER(id));
		if (session->error_message)
			g_free (session->error_message);
		session->error_message = err->message;
		session->last_err = id;
	}
#endif
	return id;
}

QofErrorId
qof_error_get_id_be (QofBackend * be)
{
	QofError * err;
	GList * first;

	g_return_val_if_fail (be, QOF_FATAL);
	if (g_list_length (be->error_stack) == 0)
		return QOF_SUCCESS;
	first = g_list_first (be->error_stack);
	err = (QofError*)first->data;
	if (!err)
		return QOF_FATAL;
	be->error_stack = 
		g_list_remove (be->error_stack, err);
#ifndef QOF_DISABLE_DEPRECATED
	set_previous_error (be);
#endif
	return err->id;
}

const gchar *
qof_error_get_message (QofSession * session)
{
	const gchar * msg;

	g_return_val_if_fail (session, NULL);
	g_return_val_if_fail (session->backend, NULL);
	msg = qof_error_get_message_be (session->backend);
#ifndef QOF_DISABLE_DEPRECATED
	{
		QofError * err;

		err = g_hash_table_lookup (error_table, 
			GINT_TO_POINTER(session->backend->last_err));
		if (session->error_message)
			g_free (session->error_message);
		session->error_message = g_strdup(msg);
		session->last_err = err->id;
	}
#endif
	return msg;
}

const gchar *
qof_error_get_message_be (QofBackend * be)
{
	QofError * err;
	GList * first;

	g_return_val_if_fail (be, NULL);
	if (g_list_length (be->error_stack) == 0)
		return NULL;
	first = g_list_first (be->error_stack);
	err = (QofError*)first->data;
	if (!err)
		return NULL;
	be->error_stack = 
		g_list_remove (be->error_stack, err);
#ifndef QOF_DISABLE_DEPRECATED
	be->error_msg = err->message;
	set_previous_error (be);
#endif
	return err->message;
}
