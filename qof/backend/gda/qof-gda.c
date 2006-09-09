/********************************************************************
 *            qof-gda.c
 *
 *  Sat Sep  9 13:11:17 2006
 *  Copyright  2006  Neil Williams
 *  linux@codehelp.co.uk
 ********************************************************************/
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
#include <glib.h>
#include <libgda/libgda.h>
#include "qof.h"

#define ACCESS_METHOD "gda"
#define QOFGDA_MODULE "qof-backend-gda"
#define LIBGDA_DIR    ".libgda"

static QofLogModule log_module = QOFGDA_MODULE;

typedef struct
{
	QofBackend be;
	GdaClient * client_pool;
	GdaConnection * connection;
	GdaCommand * command;
	GdaDataModel * dm;
	GdaValue * gda_value;
	GdaTransaction * undo_trans, * commit_trans;
	GdaError * gda_err;
	GList * entities;
	gint dbversion;
	gint create_handler;
	gint delete_handler;
	const gchar *fullpath;
	gchar *err;
	gchar *sql_str;
	gboolean error;
	QofIdType e_type;
	QofBook * book;
} QGdaBackend;

static gboolean
qgda_determine_file_type (const gchar * path)
{
	if (!path)
		return FALSE;
	return TRUE;
}

static void
qgda_modify (QofBackend *be, QofInstance *inst)
{
	
}

static void
qgda_session_begin(QofBackend *be, QofSession *session, const 
				   gchar *book_path, gboolean ignore_lock,
				   gboolean create_if_nonexistent)
{
	QGdaBackend *qgda_be;
	GList * connection_errors, *node;

	qgda_be = (QGdaBackend*)be;
	be->fullpath = g_strdup (book_path);
	if(book_path == NULL)
	{
		qof_backend_set_error(be, ERR_BACKEND_BAD_URL);
		qgda_be->error = TRUE;
		LEAVE (" bad URL");
		return;
	}
	/* check/create the ~/.libgda location. */
	{
		gchar * gdahome;
		struct stat lg;
		gint ret;

		gdahome = g_strconcat (g_get_home_dir(), 
			"/", LIBGDA_DIR, NULL);
		ret = g_stat (gdahome, &lg);
		if (ret)
		{
			qof_backend_set_error(be, ERR_BACKEND_MISC);
			qgda_be->error = TRUE;
			LEAVE (" unable to use stat");
			return;
		}
		if (!S_ISDIR (lg.st_mode) || lg.st_size == 0)
			ret = g_mkdir_with_parents (gdahome, 0700);
		if (ret)
		{
			qof_backend_set_error(be, ERR_BACKEND_MISC);
			qgda_be->error = TRUE;
			LEAVE (" unable to create '%s' 0700", gdahome);
			return;
		}
		g_free (gdahome);
	}
	/* handle create_if_nonexistent */
	/* ignore_lock ?? */
	qgda_be->connection = gda_client_open_connection 
		(qgda_be->client_pool, PACKAGE, NULL, NULL,
		GDA_CONNECTION_OPTIONS_READ_ONLY);
	connection_errors = (GList *) gda_connection_get_errors 
		(qgda_be->connection);
	for (node = g_list_first (connection_errors); node != NULL; 
			node = g_list_next (node))
	{
		qgda_be->gda_err = (GdaError *) node->data;
		DEBUG ("Error no: %ld\t", 
			gda_error_get_number (qgda_be->gda_err));
		DEBUG ("desc: %s\t", 
			gda_error_get_description (qgda_be->gda_err));
		DEBUG ("source: %s\t", 
			gda_error_get_source (qgda_be->gda_err));
		DEBUG ("sqlstate: %s\n", 
			gda_error_get_sqlstate (qgda_be->gda_err));
		qof_backend_set_error (be, ERR_BACKEND_SERVER_ERR);
		qgda_be->error = TRUE;
	}
}

static void
load_entities (gpointer value, gpointer user_data)
{
	gint column_id, row_id;
	GdaDataModel * dm;
	QGdaBackend * qgda_be;

	qgda_be = (QGdaBackend*)user_data;
	dm = (GdaDataModel*)value;
	if (!dm)
	{
		qgda_be->error = TRUE;
		DEBUG (" empty data model on load");
		return;
	}
	for (column_id = 0; column_id < gda_data_model_get_n_columns (dm);
		column_id++)
		g_print("%s\t", gda_data_model_get_column_title (dm, column_id));
	g_print("\n");
	for (row_id = 0; row_id < gda_data_model_get_n_rows (dm); row_id++) {
		for (column_id = 0; column_id < gda_data_model_get_n_columns (dm);
			 column_id++)
		{
			gchar *str;
		
			qgda_be->gda_value = (GdaValue*)gda_data_model_get_value_at 
				(dm, column_id, row_id);
			str = gda_value_stringify (qgda_be->gda_value);
			g_print ("%s\t", str);
			g_free (str);
		}
		g_print("\n");
	}
	g_object_unref(dm);
}

static void
qgda_class_foreach (QofObject * obj, gpointer data)
{
	QGdaBackend *qgda_be;

	qgda_be = (QGdaBackend*)data;
	qgda_be->sql_str = g_strdup_printf(
		"SELECT * FROM %s;", obj->e_type);
	PINFO (" sql=%s", qgda_be->sql_str);
	qgda_be->command = gda_command_new (qgda_be->sql_str,
		GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);
	qgda_be->entities = gda_connection_execute_command (qgda_be->connection,
		qgda_be->command, NULL);
	g_list_foreach (qgda_be->entities, load_entities, qgda_be);
	gda_command_free (qgda_be->command);
}

static void
qgda_db_load (QofBackend *be, QofBook *book)
{
	QGdaBackend *qgda_be;

	qgda_be = (QGdaBackend*)be;
	/* select all */
	qgda_be->book = book;
	qof_object_foreach_type(qgda_class_foreach, qgda_be);
}

static void
qgda_write_db (QofBackend *be, QofBook *book)
{
	
}

static void
qgda_session_end (QofBackend *be)
{
	QGdaBackend *qgda_be;

	qgda_be = (QGdaBackend*)be;
	if (qgda_be->dm)
		g_object_unref(G_OBJECT(qgda_be->dm));
	gda_client_close_all_connections (qgda_be->client_pool);
}

static void
qgda_destroy_backend (QofBackend *be)
{
	QGdaBackend *qgda_be;

	qgda_be = (QGdaBackend*)be;
	if (qgda_be)
		g_object_unref(G_OBJECT(qgda_be->client_pool));
	qof_event_unregister_handler (qgda_be->create_handler);
	qof_event_unregister_handler (qgda_be->delete_handler);
	g_free (be);
	g_free (qgda_be);
}

static QofBackend *
qgda_backend_new (void)
{
	QGdaBackend *qgda_be;
	QofBackend *be;

	ENTER (" ");
	qgda_be = g_new0(QGdaBackend, 1);
	be = (QofBackend*) qgda_be;
	qof_backend_init(be);
	gda_init (PACKAGE, "0.1", 0, NULL);
	qgda_be->client_pool = gda_client_new ();
	qgda_be->dbversion = QOF_OBJECT_VERSION;
	be->session_begin = qgda_session_begin;

	be->session_end = qgda_session_end;
	be->destroy_backend = qgda_destroy_backend;
	be->load = qgda_db_load;
	be->save_may_clobber_data = NULL;
	be->begin = NULL;
	/* commit: write to gda, commit undo record. */
	be->commit = qgda_modify;
	be->rollback = NULL;
	/* would need a QofQuery back to QofSqlQuery conversion. */
	be->compile_query = NULL;
	/* unused */
	be->free_query = NULL;
	be->run_query = NULL;
	be->counter = NULL;
	/* The QOF GDA backend might be multi-user */
	be->events_pending = NULL;
	be->process_events = NULL;

	be->sync = qgda_write_db;
	be->load_config = NULL;
	be->get_config = NULL;
	LEAVE (" ");
	return be;
}

static void
qgda_provider_free (QofBackendProvider *prov)
{
	prov->provider_name = NULL;
	prov->access_method = NULL;
	g_free (prov);
}

void qof_gda_provider_init(void)
{
	QofBackendProvider *prov;

	ENTER (" ");
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "QOF GDA Backend Version 0.1";
	prov->access_method = ACCESS_METHOD;
	prov->partial_book_supported = TRUE;
	prov->backend_new = qgda_backend_new;
	prov->check_data_type = qgda_determine_file_type;
	prov->provider_free = qgda_provider_free;
	qof_backend_register_provider (prov);
	LEAVE (" ");
}
