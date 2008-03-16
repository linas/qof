/*******************************************************************
 *            qsf-backend.c
 *
 *  Sat Jan  1 15:07:14 2005
 *  Copyright  2005-2008  Neil Williams
 *  linux@codehelp.co.uk
 *******************************************************************/
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
#include <sys/stat.h>
#include <glib.h>
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>
#include "qof.h"
#include "qofobject-p.h"
#include "qof-backend-qsf.h"
#include "qsf-xml.h"
#include "qsf-dir.h"

#define QSF_TYPE_BINARY "binary"
#define QSF_TYPE_GLIST  "glist"
#define QSF_TYPE_FRAME  "frame"

static QofLogModule log_module = QOF_MOD_QSF;

static void qsf_object_commitCB (gpointer key, gpointer value,
	gpointer data);

struct QSFBackend_s
{
	QofBackend be;
	QsfParam *params;
	gchar *fullpath;
};

typedef struct QSFBackend_s QSFBackend;

static void
option_cb (QofBackendOption * option, gpointer data)
{
	QsfParam *params;

	params = (QsfParam *) data;
	g_return_if_fail (params);
	if (0 == safe_strcmp (QSF_COMPRESS, option->option_name))
	{
		params->use_gz_level = (*(gint64 *) option->value);
		PINFO (" compression=%" G_GINT64_FORMAT, params->use_gz_level);
	}
	if (0 == safe_strcmp (QSF_MAP_FILES, option->option_name))
	{
		params->map_files = g_list_copy ((GList *) option->value);
	}
	if (0 == safe_strcmp (QSF_ENCODING, option->option_name))
	{
		params->encoding = g_strdup (option->value);
		PINFO (" encoding=%s", params->encoding);
	}
	if (0 == safe_strcmp (QSF_DATE_CONVERT, option->option_name))
	{
		params->convert = (*(double *) option->value);
		if (params->convert > 0)
			PINFO (" converting date into time on file write.");
	}
}

static void
qsf_load_config (QofBackend * be, KvpFrame * config)
{
	QSFBackend *qsf_be;
	QsfParam *params;

	ENTER (" ");
	qsf_be = (QSFBackend *) be;
	g_return_if_fail (qsf_be->params);
	params = qsf_be->params;
	qof_backend_option_foreach (config, option_cb, params);
	LEAVE (" ");
}

static KvpFrame *
qsf_get_config (QofBackend * be)
{
	QofBackendOption *option;
	QSFBackend *qsf_be;
	QsfParam *params;

	if (!be)
	{
		return NULL;
	}
	ENTER (" ");
	qsf_be = (QSFBackend *) be;
	g_return_val_if_fail (qsf_be->params, NULL);
	params = qsf_be->params;
	qof_backend_prepare_frame (be);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = QSF_COMPRESS;
	option->description =
		_("Level of compression to use: 0 for none, 9 for highest.");
	option->tooltip =
		_("QOF can compress QSF XML files using gzip. "
		"Note that compression is not used when outputting to STDOUT.");
	option->type = KVP_TYPE_GINT64;
	/* GINT_TO_POINTER can only be used for 32bit values. */
	option->value = (gpointer) & params->use_gz_level;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = QSF_MAP_FILES;
	option->description =
		_("List of QSF map files to use for this session.");
	option->tooltip =
		_("QOF can convert objects within QSF XML files "
		"using a map of the changes required.");
	option->type = KVP_TYPE_GLIST;
	option->value = (gpointer) params->map_files;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = QSF_ENCODING;
	option->description =
		_("Encoding string to use when writing the XML file.");
	option->tooltip =
		_("QSF defaults to UTF-8. Other encodings are supported by "
		"passing the encoding string in this option.");
	option->type = KVP_TYPE_STRING;
	option->value = (gpointer) params->encoding;
	qof_backend_prepare_option (be, option);
	g_free (option);
	option = g_new0 (QofBackendOption, 1);
	option->option_name = QSF_DATE_CONVERT;
	option->description = 
		_("Convert deprecated date values to time values.");
	option->tooltip = 
		_("Applications that support the new QOF time format "
		"need to enable this option to convert older date values into time. "
		"Applications that still use date should not set this option "
		"until time values are supported.");
	option->type = KVP_TYPE_GINT64;
	option->value = &params->convert;
	qof_backend_prepare_option (be, option);
	g_free (option);
	LEAVE (" ");
	return qof_backend_complete_frame (be);
}

GList **
qsf_map_prepare_list (GList ** maps)
{
	/* Add new map filenames here. */
	/** \todo Automate this once map support is stable */
	*maps = g_list_prepend (*maps, "pilot-qsf-GnuCashInvoice.xml");
	*maps = g_list_prepend (*maps, "pilot-qsf-gncCustomer.xml");
	return maps;
}

static void
qsf_param_init (QsfParam * params)
{
	gchar *qsf_time_string;
	gchar *qsf_enquiry_date;
	gchar *qsf_time_now;
	gchar *qsf_time_precision;

	g_return_if_fail (params != NULL);
	params->count = 0;
	params->convert = 1;
	params->use_gz_level = 0;
	params->supported_types = NULL;
	params->file_type = QSF_UNDEF;
	params->qsf_ns = NULL;
	params->output_doc = NULL;
	params->output_node = NULL;
	params->lister = NULL;
	params->full_kvp_path = NULL;
	params->map_ns = NULL;
	params->map_files = NULL;
	params->map_path = NULL;
	params->encoding = "UTF-8";
	params->qsf_object_list = NULL;
	params->qsf_parameter_hash =
		g_hash_table_new (g_str_hash, g_str_equal);
	params->qsf_default_hash = g_hash_table_new (g_str_hash, g_str_equal);
	params->qsf_define_hash = g_hash_table_new (g_str_hash, g_str_equal);
	params->qsf_calculate_hash =
		g_hash_table_new (g_str_hash, g_str_equal);
	params->referenceList = NULL;
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_STRING);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_GUID);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_BOOLEAN);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_NUMERIC);
	params->supported_types = 
		g_slist_append (params->supported_types, QOF_TYPE_TIME);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_INT32);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_INT64);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_DOUBLE);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_CHAR);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_KVP);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_COLLECT);
	params->supported_types =
		g_slist_append (params->supported_types, QOF_TYPE_CHOICE);
	qsf_time_precision = "%j";
	qsf_enquiry_date = qof_time_stamp_now ();
	qsf_time_string = qof_date_print (qof_date_get_current(), 
		QOF_DATE_FORMAT_ISO);
	qsf_time_now  = qof_time_stamp_now ();

	g_hash_table_insert (params->qsf_default_hash, "qsf_enquiry_date",
		qsf_enquiry_date);
	g_hash_table_insert (params->qsf_default_hash, "qsf_time_now",
		qof_time_get_current());
	g_hash_table_insert (params->qsf_default_hash, "qsf_time_string",
		qsf_time_string);
	/* default map files */
	params->map_files = *qsf_map_prepare_list (&params->map_files);
	params->err_nomap = qof_error_register 
	(_("The selected QSF Object file '%s' requires a "
	"map but it was not provided."), TRUE);
	params->err_overflow = qof_error_register
	(_("When converting XML strings into numbers, an "
	 "overflow has been detected. The QSF object file "
	 "'%s' contains invalid data in a field that is "
	 "meant to hold a number."), TRUE);
}

static gboolean
qsf_determine_file_type (const gchar * path)
{
	struct stat sbuf;

	if (!path)
		return TRUE;
	if (0 == safe_strcmp (path, QOF_STDOUT))
		return TRUE;
	if (stat (path, &sbuf) < 0)
	{
	/* in case the error is that the file does not exist */
		FILE * f;
		f = fopen (path, "a+");
		if (f)
		{
			fclose (f);
			return TRUE;
		}
		return FALSE;
	}
	if (sbuf.st_size == 0)
		return TRUE;
	if (is_our_qsf_object (path))
		return TRUE;
	else if (is_qsf_object (path))
		return TRUE;
	else if (is_qsf_map (path))
		return TRUE;
	return FALSE;
}

static void
qsf_session_begin (QofBackend * be, QofSession * session,
	const gchar * book_path, gboolean ignore_lock,
	gboolean create_if_nonexistent)
{
	QSFBackend *qsf_be;
	gchar *p, *path;

	PINFO (" ignore_lock=%d create_if_nonexistent=%d", ignore_lock,
		create_if_nonexistent);
	g_return_if_fail (be != NULL);
	g_return_if_fail (session);
	be->fullpath = g_strdup (book_path);
	qsf_be = (QSFBackend *) be;
	g_return_if_fail (qsf_be->params != NULL);
	qsf_be->fullpath = NULL;
	if (book_path == NULL)
	{
		/* allow use of stdout */
		qof_error_set_be (be, QOF_SUCCESS);
		return;
	}
	p = strchr (book_path, ':');
	if (p)
	{
		path = g_strdup (book_path);
		if (!g_ascii_strncasecmp (path, "file:", 5))
		{
			p = g_new0 (gchar, strlen (path) - 5 + 1);
			strcpy (p, path + 5);
		}
		qsf_be->fullpath = g_strdup (p);
		g_free (path);
	}
	else
		qsf_be->fullpath = g_strdup (book_path);
	if (create_if_nonexistent)
	{
		FILE *f;

		f = fopen (qsf_be->fullpath, "a+");
		if (f)
			fclose (f);
		else
		{
			qof_error_set_be (be, qof_error_register
			(_("could not write to '%s'. "
			 "That database may be on a read-only file system, "
			 "or you may not have write permission for the "
			"directory.\n"), TRUE));
			return;
		}
	}
	qof_error_set_be (be, QOF_SUCCESS);
}

static void
qsf_free_params (QsfParam * params)
{
	g_hash_table_destroy (params->qsf_calculate_hash);
	g_hash_table_destroy (params->qsf_default_hash);
	if (params->referenceList)
		g_list_free (params->referenceList);
	g_slist_free (params->supported_types);
	if (params->map_ns)
		xmlFreeNs (params->map_ns);
	if (params->output_doc)
		xmlFreeDoc (params->output_doc);
}

static void
qsf_session_end (QofBackend * be)
{
	QSFBackend *qsf_be;

	qsf_be = (QSFBackend *) be;
	g_return_if_fail (qsf_be != NULL);
	qsf_free_params (qsf_be->params);
	g_free (qsf_be->fullpath);
	qsf_be->fullpath = NULL;
	xmlCleanupParser ();
}

static void
qsf_destroy_backend (QofBackend * be)
{
	g_free (be);
}

static void
ent_ref_cb (QofEntity * ent, gpointer user_data)
{
	QsfParam *params;
	QofEntityReference *ref;
	void (*reference_setter) (QofEntity *, QofEntity *);
	QofEntity *reference;
	QofCollection *coll;
	QofIdType type;

	params = (QsfParam *) user_data;
	g_return_if_fail (params);
	while (params->referenceList)
	{
		ref = (QofEntityReference *) params->referenceList->data;
		if (qof_object_is_choice (ent->e_type))
			type = ref->choice_type;
		else
			type = ref->type;
		coll = qof_book_get_collection (params->book, type);
		reference = qof_collection_lookup_entity (coll, ref->ref_guid);
		reference_setter =
			(void (*)(QofEntity *, QofEntity *)) ref->param->param_setfcn;
		if (reference_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) ent, ref->param);
			qof_util_param_edit ((QofInstance *) reference, ref->param);
			reference_setter (ent, reference);
			qof_util_param_commit ((QofInstance *) ent, ref->param);
			qof_util_param_commit ((QofInstance *) reference, ref->param);
		}
		params->referenceList = g_list_next (params->referenceList);
	}
}

static void
insert_ref_cb (QofObject * obj, gpointer user_data)
{
	QsfParam *params;

	params = (QsfParam *) user_data;
	g_return_if_fail (params);
	qof_object_foreach (obj->e_type, params->book, ent_ref_cb, params);
}

/*================================================
	Load QofEntity into QofBook from XML in memory
==================================================*/

static gboolean
qsfdoc_to_qofbook (QsfParam * params)
{
	QofInstance *inst;
	struct QsfNodeIterate qiter;
	QofBook *book;
	GList *object_list;
	xmlNodePtr qsf_root;
	xmlNsPtr qsf_ns;

	g_return_val_if_fail (params != NULL, FALSE);
	g_return_val_if_fail (params->input_doc != NULL, FALSE);
	g_return_val_if_fail (params->book != NULL, FALSE);
	g_return_val_if_fail (params->file_type == OUR_QSF_OBJ, FALSE);
	qsf_root = xmlDocGetRootElement (params->input_doc);
	if (!qsf_root)
		return FALSE;
	qsf_ns = qsf_root->ns;
	qiter.ns = qsf_ns;
	book = params->book;
	params->referenceList =
		(GList *) qof_book_get_data (book, ENTITYREFERENCE);
	qsf_node_foreach (qsf_root, qsf_book_node_handler, &qiter, params);
	object_list = g_list_copy (params->qsf_object_list);
	while (object_list != NULL)
	{
		params->object_set = object_list->data;
		object_list = g_list_next (object_list);
		params->qsf_parameter_hash = params->object_set->parameters;
		if (!qof_class_is_registered (params->object_set->object_type))
			continue;
		inst =
			(QofInstance *) qof_object_new_instance (params->object_set->
			object_type, book);
		g_return_val_if_fail (inst != NULL, FALSE);
		params->qsf_ent = &inst->entity;
		g_hash_table_foreach (params->qsf_parameter_hash,
			qsf_object_commitCB, params);
	}
	qof_object_foreach_type (insert_ref_cb, params);
	qof_book_set_data (book, ENTITYREFERENCE, params->referenceList);
	return TRUE;
}

/* QofBackend routine to load from file - needs a map.
*/
static gboolean
load_qsf_object (QofBook * book, const gchar * fullpath,
				 QsfParam * params)
{
	xmlNodePtr qsf_root, map_root;
	xmlDocPtr mapDoc, foreign_doc;
	gchar *map_path, *map_file;

	map_file = params->map_path;
	mapDoc = NULL;
	/* use selected map */
	if (!map_file)
	{
		qof_error_set_be (params->be, params->err_nomap);
		return FALSE;
	}
	foreign_doc = xmlParseFile (fullpath);
	if (foreign_doc == NULL)
	{
		qof_error_set_be (params->be, qof_error_register
		(_("There was an error parsing the file '%s'.\n"), TRUE));
		return FALSE;
	}
	qsf_root = NULL;
	qsf_root = xmlDocGetRootElement (foreign_doc);
	params->qsf_ns = qsf_root->ns;
	params->book = book;
	map_path = g_strdup_printf ("%s/%s", QSF_SCHEMA_DIR, map_file);
	if (!map_path)
	{
		qof_error_set_be (params->be, params->err_nomap);
		return FALSE;
	}
	mapDoc = xmlParseFile (map_path);
	if (!mapDoc)
	{
		qof_error_set_be (params->be, params->err_nomap);
		return FALSE;
	}
	map_root = xmlDocGetRootElement (mapDoc);
	params->map_ns = map_root->ns;
	params->input_doc = qsf_object_convert (mapDoc, qsf_root, params);
	qsfdoc_to_qofbook (params);
	return TRUE;
}

static gboolean
load_our_qsf_object (const gchar * fullpath, QsfParam * params)
{
	xmlNodePtr qsf_root;

	params->input_doc = xmlParseFile (fullpath);
	if (params->input_doc == NULL)
	{
		qof_error_set_be (params->be, qof_error_register
		(_("There was an error parsing the file '%s'."), TRUE));
		return FALSE;
	}
	qsf_root = NULL;
	qsf_root = xmlDocGetRootElement (params->input_doc);
	params->qsf_ns = qsf_root->ns;
	return qsfdoc_to_qofbook (params);
}

/* Determine the type of QSF and load it into the QofBook

- is_our_qsf_object, OUR_QSF_OBJ, QSF object file using only QOF objects known
	to the calling process. No map is required.
- is_qsf_object, IS_QSF_OBJ, QSF object file that may or may not have a QSF map
	to convert external objects. This temporary type will be set to HAVE_QSF_MAP 
	if a suitable map exists, or an error value returned: ERR_QSF_NO_MAP, 
	ERR_QSF_BAD_MAP or ERR_QSF_WRONG_MAP. This allows the calling process to inform 
	the user that the QSF itself is valid but a suitable map cannot be found.
- is_qsf_map, IS_QSF_MAP, QSF map file. In the backend, this generates 
	ERR_QSF_MAP_NOT_OBJ but it can be used internally when processing maps to 
	match a QSF object.

returns NULL on error, otherwise a pointer to the QofBook. Use
the qof_book_merge API to merge the new data into the current
QofBook. 
*/
static void
qsf_file_type (QofBackend * be, QofBook * book)
{
	QSFBackend *qsf_be;
	QofErrorId parse_err;
	QsfParam *params;
	FILE *f;
	gchar *path;
	gboolean result;

	g_return_if_fail (be != NULL);
	g_return_if_fail (book != NULL);
	qsf_be = (QSFBackend *) be;
	g_return_if_fail (qsf_be != NULL);
	g_return_if_fail (qsf_be->fullpath != NULL);
	g_return_if_fail (qsf_be->params != NULL);
	parse_err = qof_error_register
		(_("There was an error parsing the file '%s'."), TRUE);
	params = qsf_be->params;
	params->book = book;
	DEBUG (" qsf_be->fullpath=%s", qsf_be->fullpath);
	path = g_strdup (qsf_be->fullpath);
	f = fopen (path, "r");
	if (!f)
		qof_error_set_be (be, qof_error_register
		(_("There was an error reading the file '%s'."), TRUE));
	else
		fclose (f);
	params->filepath = g_strdup (path);
	result = is_our_qsf_object_be (params);
	if (result)
	{
		params->file_type = OUR_QSF_OBJ;
		result = load_our_qsf_object (path, params);
		if (!result)
			qof_error_set_be (be, parse_err);
		return;
	}
	else if (is_qsf_object_be (params))
	{
		params->file_type = IS_QSF_OBJ;
		result = load_qsf_object (book, path, params);
		if (!result)
			qof_error_set_be (be, parse_err);
		return;
	}
	if (qof_error_check_be (be) == params->err_nomap)
	{
		/* usable QSF object but no map available */
		params->file_type = IS_QSF_OBJ;
		result = TRUE;
	}
	if (result == FALSE)
	{
		if (is_qsf_map_be (params))
		{
			params->file_type = IS_QSF_MAP;
			qof_error_set_be (be, qof_error_register
			(_("The selected file '%s' is a QSF map and cannot "
				"be opened as a QSF object."), TRUE));
		}
	}
}

static void
qsf_object_sequence (QofParam * qof_param, gpointer data)
{
	QsfParam *params;
	GSList *checklist, *result;

	g_return_if_fail (data != NULL);
	params = (QsfParam *) data;
	result = NULL;
	checklist = NULL;
	params->knowntype = FALSE;
	checklist = g_slist_copy (params->supported_types);
	for (result = checklist; result != NULL; result = result->next)
	{
		if (0 ==
			safe_strcmp ((QofIdType) result->data, 
			qof_param->param_type))
			params->knowntype = TRUE;
	}
	g_slist_free (checklist);
	if (0 == safe_strcmp (qof_param->param_type, params->qof_type))
	{
		params->qsf_sequence =
			g_slist_append (params->qsf_sequence, qof_param);
		params->knowntype = TRUE;
	}
	/* handle params->qof_type = QOF_TYPE_GUID and qof_param->param_type != known type */
	if (0 == safe_strcmp (params->qof_type, QOF_TYPE_GUID)
		&& (params->knowntype == FALSE))
	{
		params->qsf_sequence =
			g_slist_append (params->qsf_sequence, qof_param);
		params->knowntype = TRUE;
	}
}

/* receives each entry from supported_types in sequence
	type = qof data type from supported list
	user_data = params. Holds object type
*/
static void
qsf_supported_parameters (gpointer type, gpointer user_data)
{
	QsfParam *params;

	g_return_if_fail (user_data != NULL);
	params = (QsfParam *) user_data;
	params->qof_type = (QofIdType) type;
	params->knowntype = FALSE;
	qof_class_param_foreach (params->qof_obj_type, qsf_object_sequence,
		params);
}

static KvpValueType
qsf_to_kvp_helper (const char *type_string)
{
	if (0 == safe_strcmp (QOF_TYPE_INT64, type_string))
		return KVP_TYPE_GINT64;
	if (0 == safe_strcmp (QOF_TYPE_DOUBLE, type_string))
		return KVP_TYPE_DOUBLE;
	if (0 == safe_strcmp (QOF_TYPE_NUMERIC, type_string))
		return KVP_TYPE_NUMERIC;
	if (0 == safe_strcmp (QOF_TYPE_STRING, type_string))
		return KVP_TYPE_STRING;
	if (0 == safe_strcmp (QOF_TYPE_GUID, type_string))
		return KVP_TYPE_GUID;
	if (0 == safe_strcmp (QOF_TYPE_TIME, type_string))
		return KVP_TYPE_TIME;
	if (0 == safe_strcmp (QSF_TYPE_BINARY, type_string))
		return KVP_TYPE_BINARY;
	if (0 == safe_strcmp (QSF_TYPE_GLIST, type_string))
		return KVP_TYPE_GLIST;
	if (0 == safe_strcmp (QSF_TYPE_FRAME, type_string))
		return KVP_TYPE_FRAME;
	return 0;
}

static QofIdTypeConst
kvp_value_to_qof_type_helper (KvpValueType n)
{
	switch (n)
	{
		case KVP_TYPE_GINT64:
		{
			return QOF_TYPE_INT64;
			break;
		}
		case KVP_TYPE_DOUBLE:
		{
			return QOF_TYPE_DOUBLE;
			break;
		}
		case KVP_TYPE_NUMERIC:
		{
			return QOF_TYPE_NUMERIC;
			break;
		}
		case KVP_TYPE_STRING:
		{
			return QOF_TYPE_STRING;
			break;
		}
		case KVP_TYPE_GUID:
		{
			return QOF_TYPE_GUID;
			break;
		}
		case KVP_TYPE_BOOLEAN :
		{
			return QOF_TYPE_BOOLEAN;
			break;
		}
		case KVP_TYPE_TIME :
		{
			return QOF_TYPE_TIME;
			break;
		}
		case KVP_TYPE_BINARY:
		{
			return QSF_TYPE_BINARY;
			break;
		}
		case KVP_TYPE_GLIST:
		{
			return QSF_TYPE_GLIST;
			break;
		}
		case KVP_TYPE_FRAME:
		{
			return QSF_TYPE_FRAME;
			break;
		}
		default:
		{
			return NULL;
		}
	}
}


static void
qsf_from_kvp_helper (const gchar * path, KvpValue * content, 
					 gpointer data)
{
	QsfParam *params;
	QofParam *qof_param;
	xmlNodePtr node;
	KvpValueType n;
	gchar *full_path;

	params = (QsfParam *) data;
	qof_param = params->qof_param;
	full_path = NULL;
	g_return_if_fail (params && path && content);
	n = kvp_value_get_type (content);
	switch (n)
	{
		case KVP_TYPE_GINT64:
		case KVP_TYPE_DOUBLE:
		case KVP_TYPE_NUMERIC:
		case KVP_TYPE_STRING:
		case KVP_TYPE_GUID:
		case KVP_TYPE_TIME :
		case KVP_TYPE_BOOLEAN :
		case KVP_TYPE_BINARY:
		case KVP_TYPE_GLIST:
		{
			node =
				xmlAddChild (params->output_node,
				xmlNewNode (params->qsf_ns,
					BAD_CAST qof_param->param_type));
			xmlNodeAddContent (node,
				BAD_CAST kvp_value_to_bare_string (content));
			xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE,
				BAD_CAST qof_param->param_name);
			full_path =
				g_strconcat (params->full_kvp_path, "/", path, NULL);
			xmlNewProp (node, BAD_CAST QSF_OBJECT_KVP, BAD_CAST full_path);
			xmlNewProp (node, BAD_CAST QSF_OBJECT_VALUE,
				BAD_CAST kvp_value_to_qof_type_helper (n));
			break;
		}
		case KVP_TYPE_FRAME:
		{
			if (!params->full_kvp_path)
				params->full_kvp_path = g_strdup (path);
			else
				params->full_kvp_path = g_strconcat (params->full_kvp_path,
					"/", path, NULL);
			kvp_frame_for_each_slot (kvp_value_get_frame (content),
				qsf_from_kvp_helper, params);
			g_free (params->full_kvp_path);
			params->full_kvp_path = NULL;
			break;
		}
		default:
		{
			PERR (" unsupported value = %d", kvp_value_get_type (content));
			break;
		}
	}
}

static void
qsf_from_coll_cb (QofEntity * ent, gpointer user_data)
{
	QsfParam *params;
	QofParam *qof_param;
	xmlNodePtr node;
	gchar qsf_guid[GUID_ENCODING_LENGTH + 1];

	params = (QsfParam *) user_data;
	if (!ent || !params)
		return;
	qof_param = params->qof_param;
	guid_to_string_buff (qof_entity_get_guid (ent), qsf_guid);
	node = xmlAddChild (params->output_node, xmlNewNode (params->qsf_ns,
			BAD_CAST qof_param->param_type));
	xmlNodeAddContent (node, BAD_CAST qsf_guid);
	xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE,
		BAD_CAST qof_param->param_name);
}

/******* reference handling ***********/

static gint
qof_reference_list_cb (gconstpointer a, gconstpointer b)
{
	const QofEntityReference *aa;
	const QofEntityReference *bb;

	aa = (QofEntityReference *) a;
	bb = (QofEntityReference *) b;
	if (aa == NULL)
		return 1;
	g_return_val_if_fail ((bb != NULL), 1);
	g_return_val_if_fail ((aa->type != NULL), 1);
	if ((0 == guid_compare (bb->ent_guid, aa->ent_guid))
		&& (0 == safe_strcmp (bb->type, aa->type))
		&& (0 == safe_strcmp (bb->param->param_name,
				aa->param->param_name)))
		return 0;
	return 1;
}

static QofEntityReference *
qof_reference_lookup (GList * referenceList, QofEntityReference * find)
{
	GList *single_ref;
	QofEntityReference *ent_ref;

	if (referenceList == NULL)
		return NULL;
	g_return_val_if_fail (find != NULL, NULL);
	single_ref = NULL;
	ent_ref = NULL;
	single_ref =
		g_list_find_custom (referenceList, find, qof_reference_list_cb);
	if (single_ref == NULL)
		return ent_ref;
	ent_ref = (QofEntityReference *) single_ref->data;
	g_list_free (single_ref);
	return ent_ref;
}

static void
reference_list_lookup (gpointer data, gpointer user_data)
{
	QofEntity *ent;
	QofParam *ref_param;
	QofEntityReference *reference, *starter;
	QsfParam *params;
	const GUID *guid;
	xmlNodePtr node, object_node;
	xmlNsPtr ns;
	GList *copy_list;
	gchar qsf_guid[GUID_ENCODING_LENGTH + 1], *ref_name;

	params = (QsfParam *) user_data;
	ref_param = (QofParam *) data;
	object_node = params->output_node;
	ent = params->qsf_ent;
	ns = params->qsf_ns;
	starter = g_new0 (QofEntityReference, 1);
	starter->ent_guid = qof_entity_get_guid (ent);
	starter->type = g_strdup (ent->e_type);
	starter->param = ref_param;
	starter->ref_guid = NULL;
	copy_list = g_list_copy (params->referenceList);
	reference = qof_reference_lookup (copy_list, starter);
	g_free (starter);
	if (reference != NULL)
	{
		if ((ref_param->param_getfcn == NULL)
			|| (ref_param->param_setfcn == NULL))
			return;
		ref_name = g_strdup (reference->param->param_name);
		node =
			xmlAddChild (object_node,
			xmlNewNode (ns, BAD_CAST QOF_TYPE_GUID));
		guid_to_string_buff (reference->ref_guid, qsf_guid);
		xmlNodeAddContent (node, BAD_CAST qsf_guid);
		xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE, BAD_CAST ref_name);
		g_free (ref_name);
	}
	else
	{
		ent = (QofEntity *) ref_param->param_getfcn (ent, ref_param);
		if (!ent)
			return;
		if ((0 == safe_strcmp (ref_param->param_type, QOF_TYPE_COLLECT)) ||
			(0 == safe_strcmp (ref_param->param_type, QOF_TYPE_CHOICE)))
			return;
		node =
			xmlAddChild (object_node,
			xmlNewNode (ns, BAD_CAST QOF_TYPE_GUID));
		guid = qof_entity_get_guid (ent);
		guid_to_string_buff (guid, qsf_guid);
		xmlNodeAddContent (node, BAD_CAST qsf_guid);
		xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE,
			BAD_CAST ref_param->param_name);
	}
}

/*=====================================
	Convert QofEntity to QSF XML node
qof_param holds the parameter sequence.
=======================================*/
static void
qsf_entity_foreach (QofEntity * ent, gpointer data)
{
	QsfParam *params;
	GSList *param_list, *supported;
	GList *ref;
	xmlNodePtr node, object_node;
	xmlNsPtr ns;
	gchar *string_buffer;
	QofParam *qof_param;
	QofEntity *choice_ent;
	KvpFrame *qsf_kvp;
	QofCollection *qsf_coll;
	gint param_count;
	gboolean own_guid;
	const GUID *cm_guid;
	gchar cm_sa[GUID_ENCODING_LENGTH + 1];

	g_return_if_fail (data != NULL);
	params = (QsfParam *) data;
	param_count = ++params->count;
	ns = params->qsf_ns;
	qsf_kvp = NULL;
	own_guid = FALSE;
	choice_ent = NULL;
	object_node = xmlNewChild (params->book_node, params->qsf_ns,
		BAD_CAST QSF_OBJECT_TAG, NULL);
	xmlNewProp (object_node, BAD_CAST QSF_OBJECT_TYPE,
		BAD_CAST ent->e_type);
	string_buffer = g_strdup_printf ("%i", param_count);
	xmlNewProp (object_node, BAD_CAST QSF_OBJECT_COUNT,
		BAD_CAST string_buffer);
	g_free (string_buffer);
	param_list = g_slist_copy (params->qsf_sequence);
	while (param_list != NULL)
	{
		qof_param = (QofParam *) param_list->data;
		g_return_if_fail (qof_param != NULL);
		if (0 == safe_strcmp (qof_param->param_type, QOF_TYPE_GUID))
		{
			if (!own_guid)
			{
				cm_guid = qof_entity_get_guid (ent);
				node = xmlAddChild (object_node, xmlNewNode (ns, BAD_CAST
						QOF_TYPE_GUID));
				guid_to_string_buff (cm_guid, cm_sa);
				string_buffer = g_strdup (cm_sa);
				xmlNodeAddContent (node, BAD_CAST string_buffer);
				xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE, BAD_CAST
					QOF_PARAM_GUID);
				g_free (string_buffer);
				own_guid = TRUE;
			}
			params->qsf_ent = ent;
			params->output_node = object_node;
			ref = qof_class_get_referenceList (ent->e_type);
			if (ref != NULL)
				g_list_foreach (ref, reference_list_lookup, params);
		}
		if (0 == safe_strcmp (qof_param->param_type, QOF_TYPE_COLLECT))
		{
			qsf_coll = qof_param->param_getfcn (ent, qof_param);
			if (qsf_coll)
			{
				params->qof_param = qof_param;
				params->output_node = object_node;
				if (qof_collection_count (qsf_coll) > 0)
					qof_collection_foreach (qsf_coll, qsf_from_coll_cb,
						params);
			}
			param_list = g_slist_next (param_list);
			continue;
		}
		if (0 == safe_strcmp (qof_param->param_type, QOF_TYPE_CHOICE))
		{
			/** \todo use the reference list here. */
			choice_ent =
				(QofEntity *) qof_param->param_getfcn (ent, qof_param);
			if (!choice_ent)
			{
				param_list = g_slist_next (param_list);
				continue;
			}
			node = xmlAddChild (object_node, xmlNewNode (ns, BAD_CAST
					qof_param->param_type));
			cm_guid = qof_entity_get_guid (choice_ent);
			guid_to_string_buff (cm_guid, cm_sa);
			string_buffer = g_strdup (cm_sa);
			xmlNodeAddContent (node, BAD_CAST string_buffer);
			xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE, BAD_CAST
				qof_param->param_name);
			xmlNewProp (node, BAD_CAST "name",
				BAD_CAST choice_ent->e_type);
			g_free (string_buffer);
			param_list = g_slist_next (param_list);
			continue;
		}
		if (0 == safe_strcmp (qof_param->param_type, QOF_TYPE_KVP))
		{
			qsf_kvp =
				(KvpFrame *) qof_param->param_getfcn (ent, qof_param);
			if (kvp_frame_is_empty (qsf_kvp))
				return;
			params->qof_param = qof_param;
			params->output_node = object_node;
			kvp_frame_for_each_slot (qsf_kvp, qsf_from_kvp_helper, params);
		}
		if ((qof_param->param_setfcn != NULL)
			&& (qof_param->param_getfcn != NULL))
		{
			for (supported = g_slist_copy (params->supported_types);
				supported != NULL; supported = g_slist_next (supported))
			{
				if (0 == safe_strcmp ((const gchar *) supported->data,
						(const gchar *) qof_param->param_type))
				{
					node = xmlAddChild (object_node,
						xmlNewNode (ns, BAD_CAST qof_param->param_type));
					string_buffer =
						g_strdup (qof_util_param_to_string
						(ent, qof_param));
					xmlNodeAddContent (node, BAD_CAST string_buffer);
					xmlNewProp (node, BAD_CAST QSF_OBJECT_TYPE, BAD_CAST
						qof_param->param_name);
					g_free (string_buffer);
				}
			}
		}
		param_list = g_slist_next (param_list);
	}
}

static void
qsf_foreach_obj_type (QofObject * qsf_obj, gpointer data)
{
	QsfParam *params;
	QofBook *book;
	GSList *support;

	g_return_if_fail (data != NULL);
	params = (QsfParam *) data;
	/* Skip unsupported objects */
	if ((qsf_obj->create == NULL) || (qsf_obj->foreach == NULL))
	{
		PINFO (" qsf_obj QOF support failed %s", qsf_obj->e_type);
		return;
	}
	params->qof_obj_type = qsf_obj->e_type;
	params->qsf_sequence = NULL;
	book = params->book;
	support = g_slist_copy (params->supported_types);
	g_slist_foreach (support, qsf_supported_parameters, params);
	qof_object_foreach (qsf_obj->e_type, book, qsf_entity_foreach, params);
}

/*=====================================================
	Take a QofBook and prepare a QSF XML doc in memory
=======================================================*/
/*	QSF only uses one QofBook per file - count may be removed later. */
static xmlDocPtr
qofbook_to_qsf (QofBook * book, QsfParam * params)
{
	xmlNodePtr top_node, node;
	xmlDocPtr doc;
	gchar buffer[GUID_ENCODING_LENGTH + 1];
	const GUID *book_guid;

	g_return_val_if_fail (book != NULL, NULL);
	params->book = book;
	params->referenceList =
		g_list_copy ((GList *) qof_book_get_data (book, 
		ENTITYREFERENCE));
	doc = xmlNewDoc (BAD_CAST QSF_XML_VERSION);
	top_node = xmlNewNode (NULL, BAD_CAST QSF_ROOT_TAG);
	xmlDocSetRootElement (doc, top_node);
	xmlSetNs (top_node, xmlNewNs (top_node, BAD_CAST QSF_DEFAULT_NS,
			NULL));
	params->qsf_ns = top_node->ns;
	node =
		xmlNewChild (top_node, params->qsf_ns, BAD_CAST QSF_BOOK_TAG,
		NULL);
	params->book_node = node;
	xmlNewProp (node, BAD_CAST QSF_BOOK_COUNT, BAD_CAST "1");
	book_guid = qof_entity_get_guid ((QofEntity*)book);
	guid_to_string_buff (book_guid, buffer);
	xmlNewChild (params->book_node, params->qsf_ns,
		BAD_CAST QSF_BOOK_GUID, BAD_CAST buffer);
	params->output_doc = doc;
	params->book_node = node;
	qof_object_foreach_type (qsf_foreach_obj_type, params);
	return params->output_doc;
}

static void
write_qsf_from_book (const gchar *path, QofBook * book, 
					 QsfParam * params)
{
	xmlDocPtr qsf_doc;
	gint write_result;
	QofBackend *be;

	be = qof_book_get_backend (book);
	qsf_doc = qofbook_to_qsf (book, params);
	write_result = 0;
	PINFO (" use_gz_level=%" G_GINT64_FORMAT " encoding=%s",
		params->use_gz_level, params->encoding);
	if ((params->use_gz_level > 0) && (params->use_gz_level <= 9))
		xmlSetDocCompressMode (qsf_doc, params->use_gz_level);
	g_return_if_fail (qsf_is_valid
		(QSF_SCHEMA_DIR, QSF_OBJECT_SCHEMA, qsf_doc) == TRUE);
	write_result =
		xmlSaveFormatFileEnc (path, qsf_doc, params->encoding, 1);
	if (write_result < 0)
	{
		qof_error_set_be (be, qof_error_register
			(_("Could not write to '%s'. Check that you have "
			 "permission to write to this file and that there is "
			 "sufficient space to create it."), TRUE));
		return;
	}
	qof_object_mark_clean (book);
}

static void
write_qsf_to_stdout (QofBook * book, QsfParam * params)
{
	xmlDocPtr qsf_doc;

	qsf_doc = qofbook_to_qsf (book, params);
	g_return_if_fail (qsf_is_valid
		(QSF_SCHEMA_DIR, QSF_OBJECT_SCHEMA, qsf_doc) == TRUE);
	PINFO (" use_gz_level=%" G_GINT64_FORMAT " encoding=%s",
		params->use_gz_level, params->encoding);
	xmlSaveFormatFileEnc ("-", qsf_doc, params->encoding, 1);
	fprintf (stdout, "\n");
	qof_object_mark_clean (book);
}

static void
qsf_write_file (QofBackend * be, QofBook * book)
{
	QSFBackend *qsf_be;
	QsfParam *params;
	gchar *path;

	qsf_be = (QSFBackend *) be;
	params = qsf_be->params;
	/* if fullpath is blank, book_id was set to QOF_STDOUT */
	if (!qsf_be->fullpath || (*qsf_be->fullpath == '\0'))
	{
		write_qsf_to_stdout (book, params);
		return;
	}
	path = strdup (qsf_be->fullpath);
	write_qsf_from_book (path, book, params);
	g_free (path);
}

KvpValue *
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
		case KVP_TYPE_TIME :
		{
			QofDate *qd;
			QofTime *qt;
			KvpValue *retval;

			qd = qof_date_parse (content, QOF_DATE_FORMAT_UTC);
			if(qd)
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
		case KVP_TYPE_BOOLEAN :
		{
			gboolean val;
			val = qof_util_bool_to_int (content);
			return kvp_value_new_boolean (val);
		}
		case KVP_TYPE_BINARY:
//	      return kvp_value_new_binary(value->value.binary.data,
//                                  value->value.binary.datasize);
			break;
		case KVP_TYPE_GLIST:
//  	    return kvp_value_new_glist(value->value.list);
			break;
		case KVP_TYPE_FRAME:
//	      return kvp_value_new_frame(value->value.frame);
			break;
	}
	return NULL;
}

/* ======================================================
	Commit XML data from file to QofEntity in a QofBook
========================================================= */
void
qsf_object_commitCB (gpointer key, gpointer value, gpointer data)
{
	QsfParam *params;
	QsfObject *object_set;
	xmlNodePtr node;
	QofEntityReference *reference;
	QofEntity *qsf_ent;
	QofBook *targetBook;
	const gchar *qof_type, *parameter_name;
	QofIdType obj_type, reference_type;
	gchar *tail;
	/* cm_ prefix used for variables that hold the data to commit */
	QofNumeric cm_numeric;
	gdouble cm_double;
	gboolean cm_boolean;
	gint32 cm_i32;
	gint64 cm_i64;
	gchar cm_char, (*char_getter) (xmlNodePtr);
	GUID *cm_guid;
	KvpFrame *cm_kvp;
	KvpValue *cm_value;
	KvpValueType cm_type;
	QofSetterFunc cm_setter;
	const QofParam *cm_param;
	void (*string_setter) (QofEntity *, const gchar *);
	void (*time_setter) (QofEntity *, QofTime *);
	void (*numeric_setter) (QofEntity *, QofNumeric);
	void (*double_setter) (QofEntity *, gdouble);
	void (*boolean_setter) (QofEntity *, gboolean);
	void (*i32_setter) (QofEntity *, gint32);
	void (*i64_setter) (QofEntity *, gint64);
	void (*char_setter) (QofEntity *, gchar);

	g_return_if_fail (data && value && key);
	params = (QsfParam *) data;
	node = (xmlNodePtr) value;
	parameter_name = (const gchar *) key;
	qof_type = (gchar *) node->name;
	qsf_ent = params->qsf_ent;
	targetBook = params->book;
	obj_type =
		(gchar *) xmlGetProp (node->parent, BAD_CAST QSF_OBJECT_TYPE);
	if (0 == safe_strcasecmp (obj_type, parameter_name))
	{
		return;
	}
	cm_setter = qof_class_get_parameter_setter (obj_type, parameter_name);
	cm_param = qof_class_get_parameter (obj_type, parameter_name);
	object_set = params->object_set;
	if (safe_strcmp (qof_type, QOF_TYPE_STRING) == 0)
	{
		string_setter = (void (*)(QofEntity *, const gchar *)) cm_setter;
		if (string_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
			string_setter (qsf_ent, (gchar *) xmlNodeGetContent (node));
			qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		}
	}
	if (safe_strcmp (qof_type, QOF_TYPE_TIME) == 0)
	{
		time_setter = (void (*)(QofEntity *, QofTime*)) cm_setter;
		if (time_setter != NULL)
		{
			QofDate *qd;
			QofTime *qt;

			qd = qof_date_parse (
				(const gchar*) xmlNodeGetContent (node),
				QOF_DATE_FORMAT_UTC);
			if(qd)
			{
				qt = qof_date_to_qtime (qd);
				qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
				time_setter (qsf_ent, qt);
				qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
				qof_date_free (qd);
			}
			else
				PERR (" failed to parse date string");
		}
	}
	if ((safe_strcmp (qof_type, QOF_TYPE_NUMERIC) == 0) ||
		(safe_strcmp (qof_type, QOF_TYPE_DEBCRED) == 0))
	{
		gchar *tmp;
		numeric_setter = (void (*)(QofEntity *, QofNumeric)) cm_setter;
		tmp = (char *) xmlNodeGetContent (node);
		qof_numeric_from_string (tmp, &cm_numeric);
		g_free (tmp);
		if (numeric_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
			numeric_setter (qsf_ent, cm_numeric);
			qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		}
	}
	if (safe_strcmp (qof_type, QOF_TYPE_GUID) == 0)
	{
		cm_guid = g_new0 (GUID, 1);
		if (TRUE !=
			string_to_guid ((gchar *) xmlNodeGetContent (node), cm_guid))
		{
			qof_error_set_be (params->be, qof_error_register(
			_("The selected QSF object file '%s' contains one or "
			 "more invalid GUIDs. The file cannot be processed - "
			 "please check the source of the file and try again."),
			TRUE));
			PINFO (" string to guid conversion failed for %s:%s:%s",
				xmlNodeGetContent (node), obj_type, qof_type);
			return;
		}
		reference_type =
			(gchar *) xmlGetProp (node, BAD_CAST QSF_OBJECT_TYPE);
		if (0 == safe_strcmp (QOF_PARAM_GUID, reference_type))
		{
			qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
			qof_entity_set_guid (qsf_ent, cm_guid);
			qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		}
		else
		{
			reference = qof_entity_get_reference_from (qsf_ent, cm_param);
			if (reference)
			{
				params->referenceList =
					g_list_append (params->referenceList, reference);
			}
		}
	}
	if (safe_strcmp (qof_type, QOF_TYPE_INT32) == 0)
	{
		errno = 0;
		cm_i32 =
			(gint32) strtol ((char *) xmlNodeGetContent (node), &tail, 0);
		if (errno == 0)
		{
			i32_setter = (void (*)(QofEntity *, gint32)) cm_setter;
			if (i32_setter != NULL)
			{
				qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
				i32_setter (qsf_ent, cm_i32);
				qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
			}
		}
		else
			qof_error_set_be (params->be, params->err_overflow);
	}
	if (safe_strcmp (qof_type, QOF_TYPE_INT64) == 0)
	{
		errno = 0;
		cm_i64 = strtoll ((gchar *) xmlNodeGetContent (node), &tail, 0);
		if (errno == 0)
		{
			i64_setter = (void (*)(QofEntity *, gint64)) cm_setter;
			if (i64_setter != NULL)
			{
				qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
				i64_setter (qsf_ent, cm_i64);
				qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
			}
		}
		else
			qof_error_set_be (params->be, params->err_overflow);
	}
	if (safe_strcmp (qof_type, QOF_TYPE_DOUBLE) == 0)
	{
		errno = 0;
		cm_double = strtod ((gchar *) xmlNodeGetContent (node), &tail);
		if (errno == 0)
		{
			double_setter = (void (*)(QofEntity *, gdouble)) cm_setter;
			if (double_setter != NULL)
			{
				qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
				double_setter (qsf_ent, cm_double);
				qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
			}
		}
	}
	if (safe_strcmp (qof_type, QOF_TYPE_BOOLEAN) == 0)
	{
		if (0 == safe_strcasecmp ((gchar *) xmlNodeGetContent (node),
				QSF_XML_BOOLEAN_TEST))
			cm_boolean = TRUE;
		else
			cm_boolean = FALSE;
		boolean_setter = (void (*)(QofEntity *, gboolean)) cm_setter;
		if (boolean_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
			boolean_setter (qsf_ent, cm_boolean);
			qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		}
	}
	if (safe_strcmp (qof_type, QOF_TYPE_KVP) == 0)
	{
		cm_type =
			qsf_to_kvp_helper ((gchar *)
			xmlGetProp (node, BAD_CAST QSF_OBJECT_VALUE));
		if (!cm_type)
			return;
		qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
		cm_value =
			string_to_kvp_value ((gchar *) xmlNodeGetContent (node),
			cm_type);
		cm_kvp = (KvpFrame *) cm_param->param_getfcn (qsf_ent, cm_param);
		cm_kvp = kvp_frame_set_value (cm_kvp, (gchar *) xmlGetProp (node,
				BAD_CAST QSF_OBJECT_KVP), cm_value);
		qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		g_free (cm_value);
	}
	if (safe_strcmp (qof_type, QOF_TYPE_COLLECT) == 0)
	{
		QofCollection *qsf_coll;
		QofIdType type;
		QofEntityReference *reference;
		QofParam *copy_param;
		/* retrieve the *type* of the collection, ignore any contents. */
		qsf_coll = cm_param->param_getfcn (qsf_ent, cm_param);
		type = qof_collection_get_type (qsf_coll);
		cm_guid = g_new0 (GUID, 1);
		if (TRUE !=
			string_to_guid ((gchar *) xmlNodeGetContent (node), cm_guid))
		{
			qof_error_set_be (params->be, (qof_error_register(
			_("The selected QSF object file '%s' contains one or "
			 "more invalid 'collect' values. The file cannot be processed - "
			 "please check the source of the file and try again."),
			TRUE)));
			PINFO (" string to guid collect failed for %s",
				xmlNodeGetContent (node));
			return;
		}
		/* create a QofEntityReference with this type and GUID.
		   there is only one entity each time.
		   cm_guid contains the GUID of the reference.
		   type is the type of the reference. */
		reference = g_new0 (QofEntityReference, 1);
		reference->type = g_strdup (qsf_ent->e_type);
		reference->ref_guid = cm_guid;
		reference->ent_guid = &qsf_ent->guid;
		copy_param = g_new0 (QofParam, 1);
		copy_param->param_name = g_strdup (cm_param->param_name);
		copy_param->param_type = g_strdup (cm_param->param_type);
		reference->param = copy_param;
		params->referenceList =
			g_list_append (params->referenceList, reference);
	}
	if (safe_strcmp (qof_type, QOF_TYPE_CHAR) == 0)
	{
		char_getter = (gchar (*)(xmlNodePtr)) xmlNodeGetContent;
		cm_char = char_getter (node);
		char_setter = (void (*)(QofEntity *, gchar)) cm_setter;
		if (char_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) qsf_ent, cm_param);
			char_setter (qsf_ent, cm_char);
			qof_util_param_commit ((QofInstance *) qsf_ent, cm_param);
		}
	}
}

static QofBackend *
qsf_backend_new (void)
{
	QSFBackend *qsf_be;
	QofBackend *be;

	qsf_be = g_new0 (QSFBackend, 1);
	be = (QofBackend *) qsf_be;
	qof_backend_init (be);
	qsf_be->params = g_new0 (QsfParam, 1);
	qsf_be->params->be = be;
	qsf_param_init (qsf_be->params);
	qsf_be->be.session_begin = qsf_session_begin;

	be->session_end = qsf_session_end;
	be->destroy_backend = qsf_destroy_backend;
	be->load = qsf_file_type;
	be->save_may_clobber_data = NULL;
	/* The QSF backend will always load and save the entire QSF XML file. */
	be->begin = NULL;
	be->commit = NULL;
	be->rollback = NULL;
	/* QSF uses the built-in SQL, not a dedicated SQL server. */
	be->compile_query = NULL;
	be->free_query = NULL;
	be->run_query = NULL;
	be->counter = NULL;
	/* The QSF backend is not multi-user. */
	be->events_pending = NULL;
	be->process_events = NULL;

	be->sync = qsf_write_file;
	/* use for maps, later. */
	be->load_config = qsf_load_config;
	be->get_config = qsf_get_config;

	qsf_be->fullpath = NULL;
	return be;
}

/* The QOF method of loading each backend.
QSF is loaded as a GModule using the QOF method - QofBackendProvider.
*/
static void
qsf_provider_free (QofBackendProvider * prov)
{
	prov->provider_name = NULL;
	prov->access_method = NULL;
	g_free (prov);
}

void
qsf_provider_init (void)
{
	QofBackendProvider *prov;

	bindtextdomain (PACKAGE, LOCALE_DIR);
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "QSF Backend Version 0.4";
	prov->access_method = "file";
	prov->partial_book_supported = TRUE;
	prov->backend_new = qsf_backend_new;
	prov->check_data_type = qsf_determine_file_type;
	prov->provider_free = qsf_provider_free;
	qof_backend_register_provider (prov);
}
