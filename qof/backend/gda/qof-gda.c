/********************************************************************
 *            qof-gda.c
 *
 *  Sat Sep  9 13:11:17 2006
 *  Copyright  2006-2007  Neil Williams
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

#include "config.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <libintl.h>
#include <libgda/libgda.h>
#include "qof.h"
#include "qof-gda.h"

#define _(String) dgettext (GETTEXT_PACKAGE, String)
#define ACCESS_METHOD  "gda"
#define QOFGDA_MODULE  "qof-backend-gda"
#define LIBGDA_DIR     ".libgda"
#define GDA_DBNAME     "gda-database-name"
#define GDA_USERNAME   "gda-username"
#define GDA_PASSWORD   "gda-password"
#define GDA_DATASOURCE "qof-gda-source"

static QofLogModule log_module = QOFGDA_MODULE;

typedef struct
{
	QofBackend be;
	GdaClient * client_pool;
	GdaConnection * connection;
	GdaCommand * command;
	GdaDataModel * dm;
	GValue * gda_value;
	/* GdaTransaction is now just a string label */
	gchar * undo_trans, * commit_trans;
	GError * gda_err;
//	const GdaFieldAttributes *gda_param;
	GList * entities;
	gint dbversion;
	gint create_handler;
	gint delete_handler;
	const gchar *fullpath;
	const gchar * table_name;   /* revised each iteration. */
	GList * field_list;
	/* QofBackendOption settings: */
	gchar * data_source_name;
	gchar * provider_name;
	gchar * database_name;
	gchar * source_description;
	gchar * username;
	gchar * password;
	/* end QofBackendOption */
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
	/* accept all requests for the gda: access_method */
	return TRUE;
}

static void
qgda_modify (QofBackend *be, QofInstance *inst)
{
	
}
/*
static GdaFieldAttributes *
qoftype_to_gdafield (QofIdTypeConst qoftype)
{
	GdaFieldAttributes * p;

	p = g_new0 (GdaFieldAttributes, 1);
	p->allow_null = TRUE;
	p->gda_type = GDA_VALUE_TYPE_NULL;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_STRING))
		p->gda_type = GDA_VALUE_TYPE_STRING;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_GUID))
	{
		p->gda_type    = GDA_VALUE_TYPE_STRING;
		p->allow_null  = FALSE;
		p->primary_key = TRUE;
		p->unique_key  = TRUE;
	}
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_CHAR))
		p->gda_type =  GDA_VALUE_TYPE_STRING;
	if ((0 == safe_strcasecmp (qoftype, QOF_TYPE_DOUBLE)) ||
		(0 == safe_strcasecmp (qoftype, QOF_TYPE_NUMERIC)) ||
		(0 == safe_strcasecmp (qoftype, QOF_TYPE_DEBCRED)))
		p->gda_type =  GDA_VALUE_TYPE_DOUBLE;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_TIME))
		p->gda_type =  GDA_VALUE_TYPE_TIMESTAMP;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_BOOLEAN))
		p->gda_type =  GDA_VALUE_TYPE_BOOLEAN;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_INT32))
		p->gda_type =  GDA_VALUE_TYPE_INTEGER;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_INT64))
		p->gda_type =  GDA_VALUE_TYPE_BIGINT;
	if (0 == safe_strcasecmp (qoftype, QOF_TYPE_KVP))
		p->gda_type =  GDA_VALUE_TYPE_LIST;
	if (p->gda_type == GDA_VALUE_TYPE_NULL)
	{
		g_free (p);
		return NULL;
	}
	return p;
}
*/
/*
static void
convert_params (QofParam * param, gpointer user_data)
{
	GdaFieldAttributes * p;
	QGdaBackend * qgda_be;

	qgda_be = (QGdaBackend*)user_data;
	if (!param)
		return;
	if (0 == safe_strcasecmp (param->param_type, QOF_ID_BOOK))
		return;
	p = qoftype_to_gdafield (param->param_type);
	if (!p)
	{
		DEBUG (" unsupported QofParam: %s %s", 
			param->param_name, param->param_type);
		return;
	}
	p->name = g_strdup (param->param_name);
	p->table = g_strdup (qgda_be->table_name);
	qgda_be->field_list = g_list_append (qgda_be->field_list, p);
	PINFO (" name=%s table=%s type=%s", param->param_name,
		qgda_be->table_name, param->param_type);
}

static void
build_table (gpointer value, gpointer user_data)
{
	QGdaBackend * qgda_be;
	gint c;

	qgda_be = (QGdaBackend*)user_data;
	if (!gda_connection_is_open (qgda_be->connection))
	{
		PERR (" no connection to gda available");
		return;
	}
	PINFO (" length=%d", g_list_length(qgda_be->field_list));
	c = g_list_length(qgda_be->field_list);
	if (c > 0)
	{
		const GdaFieldAttributes *attrib[c];
		gint f;

		for (f = 0; f < c; f++)
		{
			GdaFieldAttributes * p;
			p = (GdaFieldAttributes*)qgda_be->field_list->data;
			attrib[f] = p;
			qgda_be->field_list = g_list_next (qgda_be->field_list);
		}
		gda_connection_create_table (qgda_be->connection,
			qgda_be->table_name, attrib);
	}
}
*/
static void
create_tables (QofObject * obj, gpointer user_data)
{
	QGdaBackend * qgda_be;

	qgda_be = (QGdaBackend*)user_data;
	if (qgda_be->field_list)
		g_list_free (qgda_be->field_list);
	qgda_be->field_list = NULL;
	qgda_be->table_name = obj->e_type;
//	qof_class_param_foreach (obj->e_type, convert_params, 
//		qgda_be);
//	g_list_foreach (qgda_be->field_list, build_table, qgda_be);
}

static gboolean
create_data_source (QGdaBackend * qgda_be)
{
	gchar * cnc_string;
	QofBackend * be;
	GdaProviderInfo * prov;

	ENTER (" ");
	be = (QofBackend*)qgda_be;
	if (!qgda_be->data_source_name)
	{
		qof_error_set_be (be, qof_error_register
			(_("GDA: Missing data source name."), FALSE));
		LEAVE (" empty data source name");
		return FALSE;
	}
	prov = gda_config_get_provider_by_name (qgda_be->provider_name);
	if (!prov)
	{
		gchar * msg;

		msg = g_strdup_printf (
			_("GDA Provider '%s' could not be found"),
			qgda_be->provider_name);
		qof_error_set_be (be, qof_error_register(msg, FALSE));
		g_free (msg);
		LEAVE (" provider '%s' not found", qgda_be->provider_name);
		return FALSE;
	}
/*	cnc_string = g_strconcat ("DATABASE=", qgda_be->database_name,
		NULL);*/
	cnc_string = g_strdup ("URI=/home/neil/gda-test.db");
	/* creates db within source if db does not exist */
//	gda_config_save_data_source (qgda_be->data_source_name, 
//		qgda_be->provider_name, cnc_string, 
//		qgda_be->source_description, qgda_be->username, 
//		qgda_be->password);
	/* create tables per QofObject */
	qof_object_foreach_type (create_tables, qgda_be);
	/* gda_connection_create_table (don't log password) */
	LEAVE (" created data source for %s, %s, %s, %s",
		qgda_be->data_source_name, 
		qgda_be->provider_name, cnc_string, 
		qgda_be->username);
	return TRUE;
}

static void
qgda_session_begin(QofBackend *be, QofSession *session, const 
				   gchar *book_path, gboolean ignore_lock,
				   gboolean create_if_nonexistent)
{
	QGdaBackend *qgda_be;
//	GList * connection_errors, *node;

	/* cannot use ignore_lock */
	PINFO (" gda session start");
	qgda_be = (QGdaBackend*)be;
	be->fullpath = g_strdup (book_path);
	if(book_path == NULL)
	{
		qof_error_set_be (be, 
			qof_error_register (
			_("GDA: No data source path specified."), FALSE));
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
			qof_error_set_be (be, qof_error_register
				(_("GDA: Unable to locate your home directory."),
				FALSE));
			qgda_be->error = TRUE;
			LEAVE (" unable to use stat on home_dir.");
			return;
		}
		if (!S_ISDIR (lg.st_mode) || lg.st_size == 0)
			ret = g_mkdir_with_parents (gdahome, 0700);
		if (ret)
		{
			qof_error_set_be (be, qof_error_register
				(_("GDA: Unable to create a .libgda directory "
				"within your home directory."), FALSE));
			qgda_be->error = TRUE;
			LEAVE (" unable to create '%s' 0700", gdahome);
			return;
		}
		g_free (gdahome);
	}
	{
		/* check data source */
		GdaDataSourceInfo * source;
		gboolean created;

		created = FALSE;
		source = gda_config_find_data_source
			(qgda_be->data_source_name);
		if (!source && create_if_nonexistent)
		{
			DEBUG (" no source, creating . . .");
			created = create_data_source (qgda_be);
		}
		if (!source && !created)
		{
			qof_error_set_be (be, qof_error_register
				(_("GDA: No data source found at '%s' - "
				"Try loading data from another file "
				"and write to gda: again to create the "
				"GDA data source."), TRUE));
			DEBUG (" no source but set not to create.");
			qgda_be->error = TRUE;
			return;
		}
	}
	PINFO (" trying for a connection");
	/* use the username and password that created the source */
//	qgda_be->connection = gda_client_open_connection 
//		(qgda_be->client_pool, qgda_be->data_source_name, 
//		NULL, NULL, GDA_CONNECTION_OPTIONS_DONT_SHARE);
	if (qgda_be->connection)
	{
		PINFO (" appear to be connected.");
		/* create tables per QofObject */
		qof_object_foreach_type (create_tables, qgda_be);
//		connection_errors = (GList *) gda_connection_get_errors 
//			(qgda_be->connection);
//		for (node = g_list_first (connection_errors); node != NULL; 
//				node = g_list_next (node))
//		{
//			gchar * msg;

//			msg = g_strdup_printf (
//				_("GDA encountered an error '%s' "
//				"using data source '%s'."),
//				gda_error_get_description (qgda_be->gda_err),
//				gda_error_get_source (qgda_be->gda_err));
//			qgda_be->gda_err = (GdaError *) node->data;
//			DEBUG ("Error no: %ld\t", 
//				gda_error_get_number (qgda_be->gda_err));
//			DEBUG ("desc: %s\t", 
//				gda_error_get_description (qgda_be->gda_err));
//			DEBUG ("source: %s\t", 
//				gda_error_get_source (qgda_be->gda_err));
//			DEBUG ("sqlstate: %s\n", 
//				gda_error_get_sqlstate (qgda_be->gda_err));
//			qof_error_set_be (be, qof_error_register (msg, FALSE));
//			g_free (msg);
//		}
	}
	else
	{
		PERR (" failed to connect to GDA");
		qgda_be->error = TRUE;
		qof_error_set_be (be, qof_error_register
			(_("Failed to connect to '%s'."), TRUE));
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
//			gchar *str;
		
//			qgda_be->gda_value = (GdaValue*)gda_data_model_get_value_at 
//				(dm, column_id, row_id);
//			str = gda_value_stringify (qgda_be->gda_value);
//			g_print ("%s\t", str);
//			g_free (str);
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
//	qgda_be->entities = gda_connection_execute_command (qgda_be->connection,
//		qgda_be->command, NULL);
	g_list_foreach (qgda_be->entities, load_entities, qgda_be);
	gda_command_free (qgda_be->command);
}

static void
qgda_db_load (QofBackend *be, QofBook *book)
{
	QGdaBackend *qgda_be;

	qgda_be = (QGdaBackend*)be;
	if (qgda_be->error)
		return;
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

static void
option_cb (QofBackendOption * option, gpointer data)
{
	QGdaBackend * qgda_be;

	qgda_be = (QGdaBackend *) data;
	g_return_if_fail (qgda_be);
	if (0 == safe_strcmp (GDA_DBNAME, option->option_name))
	{
		qgda_be->database_name = g_strdup (option->value);
		PINFO (" database name = %s", qgda_be->database_name);
	}
	if (0 == safe_strcmp (GDA_USERNAME, option->option_name))
	{
		qgda_be->username = g_strdup (option->value);
		PINFO (" username=%s", qgda_be->username);
	}
	if (0 == safe_strcmp (GDA_PASSWORD, option->option_name))
	{
		/* don't log the password! :-) */
		qgda_be->password = g_strdup (option->value);
	}
	if (0 == safe_strcmp (GDA_DATASOURCE, option->option_name))
	{
		qgda_be->data_source_name = g_strdup (option->value);
	}
}

static void
load_config (QofBackend * be, KvpFrame * config)
{
	QGdaBackend *qgda_be;

	ENTER (" ");
	qgda_be = (QGdaBackend *) be;
	g_return_if_fail (qgda_be);
	qof_backend_option_foreach (config, option_cb, qgda_be);
	LEAVE (" ");
}

static KvpFrame *
get_config (QofBackend * be)
{
	QofBackendOption *option;
	QGdaBackend *qgda_be;

	if (!be)
	{
		return NULL;
	}
	ENTER (" ");
	qgda_be = (QGdaBackend *) be;
	g_return_val_if_fail (qgda_be, NULL);
	qof_backend_prepare_frame (be);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = GDA_DBNAME;
	option->description =
		_("Name of the database to use.");
	option->tooltip =
		_("Override the default database name with "
		"a name of your own choice.");
	option->type = KVP_TYPE_STRING;
	option->value = (gpointer) qgda_be->database_name;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = GDA_USERNAME;
	option->description =
		_("The username to use to access this data source.");
	option->tooltip =
		_("The username specified in the configuration of this "
		"data source that provides write access to the data.");
	option->type = KVP_TYPE_STRING;
	option->value = (gpointer) qgda_be->username;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = GDA_PASSWORD;
	option->description =
		_("Password to use with the username.");
	option->tooltip =
		_("The password that is to be used with the specified "
		"username.");
	option->type = KVP_TYPE_STRING;
	option->value = (gpointer) qgda_be->password;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = GDA_DATASOURCE;
	option->description =
		_("Name of this data source.");
	option->tooltip =
		_("The name of this data source as specified "
		"in the GDA configuration.");
	option->type = KVP_TYPE_STRING;
	option->value = (gpointer) qgda_be->password;
	qof_backend_prepare_option (be, option);
	g_free (option);
	LEAVE (" ");
	return qof_backend_complete_frame (be);
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
	be->load_config = load_config;
	be->get_config = get_config;
	LEAVE (" ");

/* DEBUG */
	qgda_be->data_source_name = "QOF_DEBUG";
	qgda_be->database_name = "URI=/home/neil/test.gda";
	qgda_be->provider_name = "XML";
	qgda_be->source_description = "QOF GDA debug data";
/* end debug */
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

	bindtextdomain (PACKAGE, LOCALE_DIR);
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "QOF GDA Backend Version 0.1";
	prov->access_method = ACCESS_METHOD;
	prov->partial_book_supported = TRUE;
	prov->backend_new = qgda_backend_new;
	prov->check_data_type = qgda_determine_file_type;
	prov->provider_free = qgda_provider_free;
	qof_backend_register_provider (prov);
	qof_log_set_level (QOFGDA_MODULE, QOF_LOG_DETAIL);
}
